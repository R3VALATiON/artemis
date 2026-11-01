// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QMutex>
#include <QString>
#include <array>
#include <map>
#include <vector>
#include "nativefeedbackpolicy.h"

// One physical native controller per stream. Other controllers retain normal SDL input.
class NativeDualSense {
public:
    ~NativeDualSense();
    bool attach(int controller, const QString& serial, unsigned product);
    void detach(int controller);
    void stop();
    void tick(int mode, bool master, int bitrate, bool poor, uint64_t now);
    void acknowledge(const uint8_t* payload);
    void receive(unsigned controller, unsigned sequence, unsigned frames,
                 unsigned channels, const uint8_t* pcm, unsigned length);
    bool isBluetooth();
private:
    void closeRoute();
    void silence();
    void process(const std::vector<uint8_t>& pcm);
    bool writeBluetooth(const uint8_t* data, size_t length);
    QMutex mutex;
    NativeFeedbackPolicy policy;
    int controller = -1;
    int hid = -1;
    void* audio = nullptr;
    void* encoder = nullptr;
    bool requested = false, ackEnabled = false, playing = false, awake = false;
    uint16_t requestId = 0, expected = 0;
    bool haveSequence = false;
    uint64_t lastTick = 0, lastSend = 0, lastAck = 0, lastPcm = 0;
    std::map<uint16_t, std::vector<uint8_t>> pending;
    std::array<double,127> coefficients{}, left{}, right{};
    std::array<uint8_t,64> haptics{};
    std::array<int16_t,1024> speaker{};
    size_t ring = 0, phase = 0, position = 0, speakerFrames = 0;
    uint8_t sequence = 0, counter = 0;
};
