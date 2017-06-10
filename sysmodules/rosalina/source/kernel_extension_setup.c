/*
*   This file is part of Luma3DS
*   Copyright (C) 2016-2017 Aurora Wright, TuxSH
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
*   Additional Terms 7.b and 7.c of GPLv3 apply to this file:
*       * Requiring preservation of specified reasonable legal notices or
*         author attributions in that material or in the Appropriate Legal
*         Notices displayed by works containing it.
*       * Prohibiting misrepresentation of the origin of that material,
*         or requiring that modified versions of such material be marked in
*         reasonable ways as different from the original version.
*/

#include "kernel_extension.h"
#include "kernel_extension_setup.h"

#define MPCORE_REGS_BASE        0x17E00000
#define MPCORE_GID_REGS_BASE    (MPCORE_REGS_BASE + 0x1000)
#define MPCORE_GID_SGI          REG32(MPCORE_GID_REGS_BASE + 0xF00)

static vu8 *const kextVectorsStatus = (vu8 *)PA_PTR((void *)0x1FFFFFF0);

struct Parameters
{
    void (*SGI0HandlerCallback)(volatile struct Parameters *);
    void *interruptManager;
    u32 *L2MMUTable; // bit31 mapping

    void (*initFPU)(void);
    void (*mcuReboot)(void);
    void (*coreBarrier)(void);

    u32 TTBCR;
    u32 L1MMUTableAddrs[4];

    void **originalHandlers;
    struct CfwInfo
    {
        char magic[4];

        u8 versionMajor;
        u8 versionMinor;
        u8 versionBuild;
        u8 flags;

        u32 commitHash;

        u32 config;
    } __attribute__((packed)) info;
};
/*
static void K_JumpToKernelExtension(volatile struct Parameters *p)
{
    __asm__ volatile("cpsid aif"); // disable interruptss
    p->coreBarrier();
    __asm__ __volatile__("mcr p15, 0, %[val], c8, c7, 0" :: [val] "r" (0) : "memory"); // not necessary since that addr has never been mapped before
    __asm__ __volatile__("mcr p15, 0, %[val], c7, c10, 4" :: [val] "r" (0) : "memory");
    __asm__ __volatile__("mcr p15, 0, %[val], c7, c10, 4" :: [val] "r" (0) : "memory");
    p->coreBarrier();
}*/

static void K_MapKernelExtensionL1(volatile struct Parameters *p)
{
    u32 L1MMUTableAddr;
    vu32 *L1MMUTable;
    u32 coreId;

    __asm__ volatile("cpsid aif"); // disable interruptss
    p->coreBarrier();

    __asm__ volatile("mrc p15, 0, %0, c0, c0, 5" : "=r"(coreId));
    coreId &= 3;

    __asm__ volatile("mrc p15, 0, %0, c2, c0, 1" : "=r"(L1MMUTableAddr));
    L1MMUTableAddr &= ~0x3FFF;
    p->L1MMUTableAddrs[coreId] = L1MMUTableAddr;
    L1MMUTable = (vu32 *)(L1MMUTableAddr | (1 << 31));

    // Actually map the kernel ext
    u32 L2MMUTableAddr = (u32)(p->L2MMUTable) & ~(1 << 31);
    L1MMUTable[0x40000000 >> 20] = L2MMUTableAddr | 1;
    __asm__ __volatile__("mcr p15, 0, %[val], c8, c7, 0" :: [val] "r" (0) : "memory"); // not necessary since that addr has never been mapped before
    __asm__ __volatile__("mcr p15, 0, %[val], c7, c10, 4" :: [val] "r" (0) : "memory");
    ((void * (*)(volatile struct Parameters *))0x40000000)(p);

    p->coreBarrier();
}

u32 ALIGN(0x1000) L2MMUTableFor0x40000000[256] = { 0 };
static volatile struct Parameters *p;

static void K_MapKernelExtensionL2(void)
{
    u32 *off;
    u32 *initFPU, *mcuReboot, *coreBarrier;
    u32 TTBCR;

    // Search for stuff in the 0xFFFF0000 page
    for(initFPU = (u32 *)0xFFFF0000; initFPU < (u32 *)0xFFFF1000 && *initFPU != 0xE1A0D002; initFPU++);
    initFPU += 3;

    for(mcuReboot = initFPU; mcuReboot < (u32 *)0xFFFF1000 && *mcuReboot != 0xE3A0A0C2; mcuReboot++);
    mcuReboot--;
    coreBarrier = (u32 *)decodeARMBranch(mcuReboot - 4);

    for(off = mcuReboot; off < (u32 *)0xFFFF1000 && *off != 0x726C6468; off++); // "hdlr"

    p = (struct Parameters *)PA_FROM_VA_PTR(off); // Caches? What are caches?
    p->SGI0HandlerCallback = (void (*)(volatile struct Parameters *))PA_FROM_VA_PTR(K_MapKernelExtensionL1);
    p->L2MMUTable = (u32 *)PA_FROM_VA_PTR(L2MMUTableFor0x40000000);
    p->initFPU = (void (*) (void))initFPU;
    p->mcuReboot = (void (*) (void))mcuReboot;
    p->coreBarrier = (void (*) (void))coreBarrier;
    __asm__ volatile("mrc p15, 0, %0, c2, c0, 2" : "=r"(TTBCR));
    p->TTBCR = TTBCR;

    // Now let's configure the L2 table

    //4KB extended small pages: [SYS:RW USR:-- X  TYP:NORMAL SHARED OUTER NOCACHE, INNER CACHED WB WA]
    for(u32 offset = 0; offset < kernel_extension_size; offset += 0x1000)
        L2MMUTableFor0x40000000[offset >> 12] = (u32)convertVAToPA(kernel_extension + offset) | 0x516;

    __asm__ __volatile__("mcr p15, 0, %[val], c7, c10, 4" :: [val] "r" (0) : "memory");
}

static void K_SendSGI0ToAllCores(void)
{
    MPCORE_GID_SGI = 0xF0000; // http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0360f/CACGDJJC.html
}

static inline void flushAllCaches(void)
{
    svcUnmapProcessMemory(CUR_PROCESS_HANDLE, 0, 0); // this SVC flush both caches entirely (and properly) even when returing an error
}

void installKernelExtension(void)
{
    svc0x2F(K_MapKernelExtensionL2);
    flushAllCaches();
    svc0x2F(K_SendSGI0ToAllCores);
    flushAllCaches();
    /*svc0x2F(K_PrepareToJumpToKernelExtension);
    flushAllCaches();
    svc0x2F(K_SendSGI0ToAllCores);
    flushAllCaches();*/

    *kextVectorsStatus |= 5 << 2; // SVC handler ready | kext ready
}
