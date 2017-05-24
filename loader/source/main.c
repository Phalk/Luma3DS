/*
*   This file is part of Luma3DS
*   Copyright (C) 2016 Aurora Wright, TuxSH
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

#include "memory.h"
#include "cache.h"
#include "firm.h"

void main(int argc, char **argv)
{
    Firm *firm = (Firm *)0x20001000;
    char *argvPassed[2],
         absPath[24 + 255];
    struct fb fbs[2];

    if(argc > 0)
    {
        u32 i;
        for(i = 0; i < sizeof(absPath) - 1 && argv[0][i] != 0; i++)
            absPath[i] = argv[0][i];
        absPath[i] = 0;

        argvPassed[0] = (char *)absPath;
    }

    if(argc == 2)
    {
        struct fb *fbsrc = (struct fb *)argv[1];

        fbs[0] = fbsrc[0];
        fbs[1] = fbsrc[1];

        argvPassed[1] = (char *)&fbs;
    }

    launchFirm(firm, argc, argvPassed);
}
