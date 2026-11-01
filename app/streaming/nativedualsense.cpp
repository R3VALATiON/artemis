// SPDX-License-Identifier: GPL-3.0-or-later
#include "nativedualsense.h"
#include "dualsensewire.h"
#include "SDL_compat.h"
#include <Limelight.h>
#include <QDir>
#include <QFile>
#include <QMutexLocker>
#include <QtGlobal>
#include <cmath>
#include <cstring>
#include <algorithm>
#ifdef Q_OS_LINUX
#include <alsa/asoundlib.h>
#include <opus.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#endif
extern "C" int LiSendRawControlStreamPacket(uint16_t, const void*, int);

namespace {
QString readText(const QString& path) {
    QFile f(path); return f.open(QIODevice::ReadOnly) ? QString::fromUtf8(f.readAll()).trimmed() : QString();
}
QString normalSerial(QString s) { return s.remove(':').remove('-').toLower(); }
}
NativeDualSense::~NativeDualSense() { stop(); }

bool NativeDualSense::attach(int index, const QString& serial, unsigned product) {
    QMutexLocker lock(&mutex);
    if (controller >= 0) return false; // Never route a second pad's feedback to the first.
#ifdef Q_OS_LINUX
    QStringList candidates;
    const auto ids = QDir("/sys/class/hidraw").entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& node : ids) {
        const QString text = readText("/sys/class/hidraw/"+node+"/device/uevent");
        QString id, unique;
        for (const QString& line : text.split('\n')) {
            if (line.startsWith("HID_ID=")) id=line.mid(7);
            if (line.startsWith("HID_UNIQ=")) unique=line.mid(9);
        }
        const auto fields = id.split(':');
        if (fields.size()!=3 || fields[0].toUInt(nullptr,16)!=5 ||
            fields[1].toUInt(nullptr,16)!=0x054c || fields[2].toUInt(nullptr,16)!=product) continue;
        if (!serial.isEmpty() && normalSerial(unique)!=normalSerial(serial)) continue;
        candidates << "/dev/"+node;
    }
    if (candidates.size()==1) {
        hid=::open(candidates[0].toLocal8Bit().constData(),O_WRONLY|O_NONBLOCK|O_CLOEXEC);
        if (hid<0) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"DualSense Bluetooth: cannot open hidraw; see LINUX_DUALSENSE.md permissions");
            return false;
        }
        int error=0;
        encoder=opus_encoder_create(48000,2,OPUS_APPLICATION_AUDIO,&error);
        if (!encoder || error!=OPUS_OK) { closeRoute(); return false; }
        opus_encoder_ctl(static_cast<OpusEncoder*>(encoder),OPUS_SET_BITRATE(160000));
        opus_encoder_ctl(static_cast<OpusEncoder*>(encoder),OPUS_SET_VBR(0));
        opus_encoder_ctl(static_cast<OpusEncoder*>(encoder),OPUS_SET_COMPLEXITY(0));
        opus_encoder_ctl(static_cast<OpusEncoder*>(encoder),OPUS_SET_EXPERT_FRAME_DURATION(OPUS_FRAMESIZE_10_MS));
        double sum=0;
        const double pi=std::acos(-1.0), cutoff=1400.0/48000.0;
        for (size_t i=0;i<coefficients.size();++i) {
            double x=double(i)-63.0;
            double sinc=x==0 ? 2*cutoff : std::sin(2*pi*cutoff*x)/(pi*x);
            coefficients[i]=sinc*(0.42-0.5*std::cos(2*pi*i/126)+0.08*std::cos(4*pi*i/126));
            sum+=coefficients[i];
        }
        for (double& c : coefficients) c/=sum;
        controller=index;
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"DualSense native Bluetooth route ready (player %d)",index);
        return true;
    }
    // ALSA hardware devices preserve the exact four-channel order. Never select
    // a desktop's stereo default device, which could play haptics on speakers.
    QStringList cards;
    const auto nodes=QDir("/sys/class/sound").entryList(QStringList()<<"card*",QDir::Dirs);
    for (const QString& node : nodes) {
        const QString devicePath=QDir("/sys/class/sound/"+node+"/device").canonicalPath();
        if(devicePath.isEmpty()) continue;
        QDir parent(devicePath);
        bool matches=false;
        for (int depth=0;depth<8 && !parent.isRoot();++depth) {
            if (readText(parent.filePath("idVendor")).toUInt(nullptr,16)==0x054c &&
                readText(parent.filePath("idProduct")).toUInt(nullptr,16)==product) {
                const QString usbSerial=readText(parent.filePath("serial"));
                matches=serial.isEmpty() || usbSerial.isEmpty() || normalSerial(usbSerial)==normalSerial(serial);
                break;
            }
            parent.cdUp();
        }
        if(matches) cards<<node.mid(4);
    }
    if (cards.size()==1) {
        snd_pcm_t* pcm=nullptr;
        const QByteArray device=("hw:"+cards[0]+",0").toLocal8Bit();
        int result=snd_pcm_open(&pcm,device.constData(),SND_PCM_STREAM_PLAYBACK,SND_PCM_NONBLOCK);
        if(result>=0) result=snd_pcm_set_params(pcm,SND_PCM_FORMAT_S16_LE,SND_PCM_ACCESS_RW_INTERLEAVED,4,48000,0,20000);
        if(result>=0) {
            audio=pcm; controller=index;
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"DualSense native USB four-channel ALSA route ready (player %d)",index);
            return true;
        }
        if(pcm) snd_pcm_close(pcm);
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"DualSense native USB audio unavailable: %s",snd_strerror(result));
    }
#else
    Q_UNUSED(index); Q_UNUSED(serial); Q_UNUSED(product);
#endif
    return false;
}

bool NativeDualSense::isBluetooth() { QMutexLocker lock(&mutex); return hid>=0; }

bool NativeDualSense::writeBluetooth(const uint8_t* data,size_t length) {
#ifdef Q_OS_LINUX
    if (hid<0) return false;
    const auto written=::write(hid,data,length);
    if (written==ssize_t(length)) return true;
    // Never accumulate latency on a blocked Bluetooth output queue.
    if(written<0 && (errno==EAGAIN || errno==EWOULDBLOCK || errno==EINTR)) return false;
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"DualSense Bluetooth route disconnected");
    closeRoute();
#else
    Q_UNUSED(data); Q_UNUSED(length);
#endif
    return false;
}

void NativeDualSense::silence() {
#ifdef Q_OS_LINUX
    if (audio) { snd_pcm_drop(static_cast<snd_pcm_t*>(audio)); snd_pcm_prepare(static_cast<snd_pcm_t*>(audio)); }
    if (hid>=0 && encoder) {
        int16_t zero[960]{}; uint8_t encoded[200]{}; uint8_t h[64]{};
        if (opus_encode(static_cast<OpusEncoder*>(encoder),zero,480,encoded,200)==200) {
            const auto r=DualSenseWire::media(h,encoded,sequence++,counter++);
            writeBluetooth(r.data(),r.size());
        }
    }
    if(encoder) opus_encoder_ctl(static_cast<OpusEncoder*>(encoder),OPUS_RESET_STATE);
#endif
    pending.clear(); haveSequence=false; playing=false; awake=false;
    left.fill(0); right.fill(0); haptics.fill(0); speaker.fill(0);
    ring=phase=position=speakerFrames=0;
}
void NativeDualSense::closeRoute() {
#ifdef Q_OS_LINUX
    if(audio) snd_pcm_close(static_cast<snd_pcm_t*>(audio));
    if(hid>=0) ::close(hid);
    if(encoder) opus_encoder_destroy(static_cast<OpusEncoder*>(encoder));
#endif
    audio=nullptr; hid=-1; encoder=nullptr; controller=-1; awake=false;
}
void NativeDualSense::stop() {
    QMutexLocker lock(&mutex);
    silence(); closeRoute(); policy.reset(); ackEnabled=requested=false;
    lastTick=lastSend=lastAck=lastPcm=0;
}
void NativeDualSense::detach(int index) {
    QMutexLocker lock(&mutex);
    if(controller!=index) return;
    silence(); closeRoute(); ackEnabled=false;
}

void NativeDualSense::tick(int mode,bool master,int bitrate,bool poor,uint64_t now) {
    QMutexLocker lock(&mutex);
    if(lastTick && now-lastTick<500) return;
    lastTick=now;
    uint32_t rtt=0,variance=0;
    int latency=LiGetEstimatedRttInfo(&rtt,&variance) ? int(rtt) : -1;
    const bool enabled=policy.evaluate(mode,master && controller>=0,bitrate,poor,latency,int64_t(now));
    if (!lastSend || now-lastSend>=2000 || enabled!=requested) {
        requested=enabled; ++requestId;
        const auto p=DualSenseWire::policy(enabled,requestId);
        LiSendRawControlStreamPacket(0x5506,p.data(),int(p.size()));
        lastSend=now;
    }
    const bool allowed=requested && ackEnabled && lastAck && now-lastAck<6000;
    if(playing && (!allowed || (lastPcm && now-lastPcm>250))) silence();
}
void NativeDualSense::acknowledge(const uint8_t* p) {
    QMutexLocker lock(&mutex);
    if(p[0]!=0x50 || p[1]!=1 || p[3]!=0 || uint16_t(p[4]|(p[5]<<8))!=requestId) return;
    ackEnabled=p[2]!=0; lastAck=SDL_GetTicks64();
}
void NativeDualSense::receive(unsigned index,unsigned seq,unsigned frames,unsigned channels,
                             const uint8_t* pcm,unsigned length) {
    if(!pcm || !DualSenseWire::validPcm(index,frames,channels,length)) return;
    QMutexLocker lock(&mutex);
    const auto now=SDL_GetTicks64();
    if(int(index)!=controller || !requested || !ackEnabled || !lastAck || now-lastAck>=6000) return;
    if(!haveSequence || now-lastPcm>250) { pending.clear(); expected=uint16_t(seq); haveSequence=true; }
    lastPcm=now;
    const uint16_t distance=uint16_t(seq-expected);
    if(distance>=0x8000) return;
    pending.emplace(uint16_t(seq),std::vector<uint8_t>(pcm,pcm+length));
    if(!pending.count(expected) && pending.size()>=4) {
        auto nearest=std::min_element(pending.begin(),pending.end(),[this](const auto& a,const auto& b) {
            return uint16_t(a.first-expected)<uint16_t(b.first-expected);
        });
        expected=nearest->first;
    }
    while(pending.count(expected)) {
        auto block=std::move(pending[expected]); pending.erase(expected++);
        process(block);
    }
}

void NativeDualSense::process(const std::vector<uint8_t>& bytes) {
#ifdef Q_OS_LINUX
    playing=true;
    if(audio) {
        const auto result=snd_pcm_writei(static_cast<snd_pcm_t*>(audio),bytes.data(),bytes.size()/8);
        if(result==-EPIPE || result==-ESTRPIPE) snd_pcm_prepare(static_cast<snd_pcm_t*>(audio));
        else if(result<0 && result!=-EAGAIN) { closeRoute(); }
        return;
    }
    if(hid<0 || !encoder) return;
    if(!awake) {
        const auto wake=DualSenseWire::wake(sequence++);
        if(!writeBluetooth(wake.data(),wake.size())) return;
        awake=true;
    }
    for(size_t frame=0;frame<bytes.size()/8;++frame) {
        int16_t samples[4];
        for(size_t c=0;c<4;++c) samples[c]=int16_t(uint16_t(bytes[frame*8+c*2])|(uint16_t(bytes[frame*8+c*2+1])<<8));
        speaker[speakerFrames*2]=samples[0]; speaker[speakerFrames*2+1]=samples[1]; ++speakerFrames;
        left[ring]=samples[2]; right[ring]=samples[3]; ring=(ring+1)%127;
        if(++phase==16) {
            phase=0;
            for(const auto* channel : {&left,&right}) {
                double sum=0; size_t k=(ring+126)%127;
                for(double c : coefficients) { sum+=(*channel)[k]*c; k=(k+126)%127; }
                haptics[position++]=uint8_t(int8_t(std::max(-128L,std::min(127L,std::lround(sum*254.0/32768.0)))));
            }
        }
        if(speakerFrames==512) {
            int16_t converted[960]; uint8_t encoded[200];
            for(int f=0;f<480;++f) {
                int source=f*16/15,fraction=f*16%15;
                for(int c=0;c<2;++c) converted[f*2+c]=int16_t((int(speaker[source*2+c])*(15-fraction)+int(speaker[std::min(source+1,511)*2+c])*fraction)/15);
            }
            if(opus_encode(static_cast<OpusEncoder*>(encoder),converted,480,encoded,200)==200) {
                const auto r=DualSenseWire::media(haptics.data(),encoded,sequence++,counter++);
                writeBluetooth(r.data(),r.size());
            }
            speakerFrames=position=0;
            if(hid<0) return;
        }
    }
#else
    Q_UNUSED(bytes);
#endif
}
