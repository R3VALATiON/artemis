// SPDX-License-Identifier: GPL-3.0-or-later
#include "../../app/streaming/nativefeedbackpolicy.h"
#include "../../app/streaming/dualsensewire.h"
#include <cassert>
#include <iostream>
int main() {
    NativeFeedbackPolicy p;
    assert(!p.evaluate(0,true,12000,false,30,0));
    assert(!p.evaluate(0,true,12000,false,30,19999));
    assert(p.evaluate(0,true,12000,false,30,20000));
    assert(p.evaluate(0,true,12000,false,149,21000));
    assert(!p.evaluate(0,true,12000,false,150,22000));
    assert(!p.evaluate(0,true,12000,false,120,25000));
    assert(!p.evaluate(0,true,12000,false,119,26000));
    assert(!p.evaluate(0,true,12000,false,119,45999));
    assert(p.evaluate(0,true,12000,false,119,46000));
    assert(!p.evaluate(0,true,11999,false,30,47000));
    assert(!p.evaluate(0,true,12000,true,30,70000));
    assert(p.evaluate(1,true,1000,true,500,71000));
    assert(!p.evaluate(1,false,1000,true,500,72000));
    assert(!p.evaluate(2,true,50000,false,30,73000));
    assert(!p.evaluate(99,true,12000,false,-1,74000));
    assert(p.evaluate(99,true,12000,false,-1,94000));
    assert(!p.evaluate(0,true,12000,false,30,100)); // clock regression
    auto lease=DualSenseWire::policy(true,0xbeef);
    const std::array<uint8_t,8> expected{{1,1,0x70,0x17,0xef,0xbe,0,0}};
    assert(lease==expected); assert(DualSenseWire::policy(false,0)[1]==0);
    assert(DualSenseWire::validPcm(0,144,4,1152));
    assert(DualSenseWire::validPcm(15,512,4,4096));
    assert(!DualSenseWire::validPcm(16,144,4,1152));
    assert(!DualSenseWire::validPcm(0,0,4,0));
    assert(!DualSenseWire::validPcm(0,513,4,4104));
    assert(!DualSenseWire::validPcm(0,144,2,576));
    assert(!DualSenseWire::validPcm(0,144,4,1151));
    auto w=DualSenseWire::wake(3);
    uint32_t crc=uint32_t(w[138])|(uint32_t(w[139])<<8)|(uint32_t(w[140])<<16)|(uint32_t(w[141])<<24);
    assert(crc==0xff5fd553u); // independent Python zlib golden reference
    assert(DualSenseWire::wake(19)==w);
    uint8_t h[64]={}; uint8_t o[200]={}; h[63]=0x81; o[199]=0x42;
    auto m=DualSenseWire::media(h,o,17,255);
    assert(m[0]==0x35 && m[1]==0x10 && m[10]==255);
    assert(m[12]==200 && m[212]==0x42 && m[214]==64 && m[278]==0x81);
    auto original=m; m[13]^=1; DualSenseWire::crc(m.data(),m.size());
    assert(m[330]!=original[330] || m[331]!=original[331] || m[332]!=original[332] || m[333]!=original[333]);
    std::cout<<"Native feedback policy, lease, packet validation and Bluetooth CRC tests passed\n";
}
