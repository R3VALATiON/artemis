// SPDX-License-Identifier: GPL-3.0-or-later
// Same encrypted extension sender used by the personal Android preview.
#include "moonlight-common-c/src/ControlStream.c"
int LiSendRawControlStreamPacket(uint16_t packetType, const void* data, int length) {
    if (AppVersionQuad[0] < 5 || !encryptedControlStream || !data || length <= 0 || length > 252) return -1;
    return sendMessageAndForget((short)packetType, (short)length, data,
                               CTRL_CHANNEL_GENERIC, 0, false) != 0 ? 0 : -1;
}
