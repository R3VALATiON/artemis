// SPDX-License-Identifier: GPL-3.0-or-later
// Native Bluetooth framing ported from the credited Android Extended builders.
#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
#include <algorithm>
namespace DualSenseWire {
inline void crc(uint8_t* p, size_t size) {
    uint32_t c = 0xffffffff;
    auto feed = [&c](uint8_t b) {
        c ^= b;
        for (int i = 0; i < 8; ++i) c = (c >> 1) ^ (0xedb88320u & (0u - (c & 1u)));
    };
    feed(0xa2);
    for (size_t i = 0; i < size - 4; ++i) feed(p[i]);
    c = ~c;
    for (int i = 0; i < 4; ++i) p[size - 4 + i] = uint8_t(c >> (8 * i));
}
inline std::array<uint8_t, 142> wake(uint8_t sequence) {
    std::array<uint8_t, 142> r{};
    r[0]=0x32; r[1]=(sequence & 15)<<4; r[2]=0x90; r[3]=63;
    r[4]=0xfd; r[5]=0xf7; r[8]=r[9]=0x7f; r[10]=0xff;
    r[11]=9; r[13]=15; r[39]=10; r[40]=7; r[43]=2;
    r[44]=1; r[46]=0xff; r[47]=0xd7;
    crc(r.data(), r.size()); return r;
}
inline std::array<uint8_t, 334> media(const uint8_t* haptics, const uint8_t* opus,
                                      uint8_t sequence, uint8_t counter) {
    std::array<uint8_t, 334> r{};
    r[0]=0x35; r[1]=(sequence & 15)<<4; r[2]=0x91; r[3]=7; r[4]=0xfe;
    std::fill(r.begin()+5, r.begin()+10, 64); r[10]=counter;
    r[11]=0x93; r[12]=200; std::copy(opus, opus+200, r.begin()+13);
    r[213]=0x92; r[214]=64; std::copy(haptics, haptics+64, r.begin()+215);
    crc(r.data(),r.size()); return r;
}
inline bool validPcm(unsigned controller, unsigned frames, unsigned channels, unsigned length) {
    return controller < 16 && channels == 4 && frames > 0 && frames <= 512 && length == frames*8;
}
inline std::array<uint8_t,8> policy(bool on, uint16_t id) {
    return {{1,uint8_t(on),0x70,0x17,uint8_t(id),uint8_t(id>>8),0,0}};
}
}
