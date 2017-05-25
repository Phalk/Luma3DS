/*
*   This file is part of Luma3DS
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

#pragma once

#include <stdint.h>
#include <stdbool.h>

//Common data types
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef volatile u8 vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;

#define ARM11_PARAMETERS_ADDRESS        0x1FFFF000

#define GPU_POWER_CTRL                  (*(vu32 *)0x10400030)

#define CFG11_GPU_STATUS                (*(vu16 *)0x10141000)
#define CFG11_GPU_CNT                   (*(vu32 *)0x10141200)
#define CFG11_GPU_CNT2                  (*(vu8  *)0x10141204)
#define CFG11_GPU_CNT3                  (*(vu8  *)0x10141208)
#define CFG11_GPU_FCRAM_CNT             (*(vu16 *)0x10141210)

#define GPIO_DATA3                      (*(vu16 *)0x10147020)
#define GPIO_DATA3_INT_MASK             (*(vu16 *)0x10147022)

#define MPCORE_REGS_BASE                0x17E00000
#define MPCORE_GID_REGS_BASE            (MPCORE_REGS_BASE + 0x1000)

#define MPCORE_CPU_IFACE_CTRL           (*(vu32 *)(MPCORE_REGS_BASE + 0x100))
#define MPCORE_CPU_IFACE_INT_ACK        (*(vu32 *)(MPCORE_REGS_BASE + 0x10C))

#define MPCORE_GID_CTRL                 (*(vu32 *)(MPCORE_GID_REGS_BASE + 0x000))
#define MPCORE_GID_INT_ENABLE_SET       ((vu32 *)(MPCORE_GID_REGS_BASE + 0x100))
#define MPCORE_GID_INT_ENABLE_CLEAR     ((vu32 *)(MPCORE_GID_REGS_BASE + 0x180))
#define MPCORE_GID_INT_PENDING_SET      ((vu32 *)(MPCORE_GID_REGS_BASE + 0x200))
#define MPCORE_GID_INT_PENDING_CLEAR    ((vu32 *)(MPCORE_GID_REGS_BASE + 0x280))
#define MPCORE_GID_INT_PRIORITY         ((vu32 *)(MPCORE_GID_REGS_BASE + 0x400))
#define MPCORE_GID_INT_TARGETS          ((vu32 *)(MPCORE_GID_REGS_BASE + 0x800))
#define MPCORE_GID_SGI                  (*(vu32 *)(MPCORE_GID_REGS_BASE + 0xF00))

typedef enum
{
    INIT_SCREENS_SEQUENCE = 0,
    SETUP_FRAMEBUFFERS,
    CLEAR_SCREENS,
    SWAP_FRAMEBUFFERS,
    UPDATE_BRIGHTNESS,
    DEINIT_SCREENS,
    PREPARE_ARM11_FOR_FIRMLAUNCH,
    ARM11_READY,
} Arm11Operation;
