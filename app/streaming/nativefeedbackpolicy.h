// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>

// Mirrors the Android preview. Quality adaptation never changes controller identity.
class NativeFeedbackPolicy {
public:
    bool evaluate(int mode, bool available, int bitrate, bool poor, int rtt, int64_t now) {
        if (!available || mode == 2) { reset(); return false; }
        if (mode == 1) { reset(); return true; }
        if (bitrate < 12000 || poor || rtt >= 150) { reset(); return false; }
        if (rtt >= 120 && !enabled) { healthy = -1; return false; }
        if (healthy < 0 || now < healthy) healthy = now;
        if (now - healthy < 20000) return false;
        return enabled = true;
    }
    void reset() { healthy = -1; enabled = false; }
private:
    int64_t healthy = -1;
    bool enabled = false;
};
