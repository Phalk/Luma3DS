/**   This file is part of Luma3DS
*   Copyright (C) 2017 Aurora Wright, TuxSH
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*   Additional Terms 7.b of GPLv3 applies to this file: Requiring preservation of specified
*   reasonable legal notices or author attributions in that material or in the Appropriate Legal
*   Notices displayed by works containing it.
*/

#include "interrupts.h"

void disableInterrupt(u32 id)
{
    if(id < 0x80) MPCORE_GID_INT_ENABLE_CLEAR[id / 32] |= 1 << (id % 32);
    if(id < 0x20) MPCORE_GID_INT_PRIORITY[id / 4] |= 0xF << (4 + 4 * (id % 4)); // private interrupt
}

void discardInterrupt(u32 id)
{
    if(id < 0x80) MPCORE_GID_INT_PENDING_CLEAR[id / 32] |= 1 << (id % 32);
}

void configureAndEnableInterrupt(u32 id, u32 targetMask, u32 priority)
{
    if(id < 0x80)
    {
        u32 tmp = MPCORE_GID_INT_PRIORITY[id / 4] & ~(0xF << (4 + 4 * (id % 4)));
        MPCORE_GID_INT_PRIORITY[id / 4] = tmp | ((priority & 0xF) << (4 + 4 * (id % 4)));

        if(id >= 0x20)
        {
            tmp = MPCORE_GID_INT_TARGETS[id / 4] & ~(0xF << (4 * (id % 4))); 
            MPCORE_GID_INT_TARGETS[id / 4] = tmp | ((targetMask & 0xF) << (4 * (id % 4)));
        }

        MPCORE_GID_INT_ENABLE_SET[id / 32] |= 1 << (id % 32);
    }
}

u32 acknowledgeInterrupt(void)
{
    return MPCORE_CPU_IFACE_INT_ACK;
}

bool isInterruptPending(u32 id)
{
    return id < 0x80 && (MPCORE_GID_INT_PENDING_SET[id / 32] & (1 << (id % 32))) != 0;
}
