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

/*
*   Screen init code by dark_samus, bil1s, Normmatt, delebile and others
*   LCD deinit code by tiniVi
*/

#include "types.h"
#include "memory.h"
#include "screen.h"
#include "interrupts.h"

void prepareForFirmlaunch(void);
extern u32 prepareForFirmlaunchSize;

extern volatile Arm11Operation operation;

void main(void)
{
    MPCORE_GID_CTRL = 1; //Enable interrupts globally
    MPCORE_CPU_IFACE_CTRL = 1; //Enable interrupts for this core

    operation = ARM11_READY;

    while(true)
    {
        switch(operation)
        {
            case ARM11_READY:
                continue;
            case INIT_SCREENS_SEQUENCE:
                initScreensSequence(*(vu32 *)ARM11_PARAMETERS_ADDRESS);
                break;
            case SETUP_FRAMEBUFFERS:
                setupFramebuffers((struct fb *)ARM11_PARAMETERS_ADDRESS);
                break;
            case CLEAR_SCREENS:
                clearScreens((struct fb *)ARM11_PARAMETERS_ADDRESS);
                break;
            case SWAP_FRAMEBUFFERS:
                swapFramebuffers(*(volatile bool *)ARM11_PARAMETERS_ADDRESS);
                break;
            case UPDATE_BRIGHTNESS:
                updateBrightness(*(vu32 *)ARM11_PARAMETERS_ADDRESS);
                break; 
            case DEINIT_SCREENS:
                deinitScreens();
                break;
            case PREPARE_ARM11_FOR_FIRMLAUNCH:
                memcpy((void *)0x1FFFFC00, (void *)prepareForFirmlaunch, prepareForFirmlaunchSize);
                *(vu32 *)0x1FFFFFFC = 0;
                ((void (*)(u32, volatile Arm11Operation *))0x1FFFFC00)(ARM11_READY, &operation);
        }

        operation = ARM11_READY;
    }
}
