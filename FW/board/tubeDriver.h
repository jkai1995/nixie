#ifndef TUBEDRIVER_H
#define TUBEDRIVER_H

#include "stm32f10x.h"
typedef int (*pfnTubeIndex)(u32);
typedef int (*pfnTubeIndexValue)(u32,s8);
typedef int (*pfnTube)();
typedef void (*pfnTubeVoid)();

typedef struct
{
    pfnTubeIndex        clearTube;
    pfnTubeIndexValue   setTube;
    pfnTube             setDote;
    pfnTube             clearDote;
    pfnTubeVoid         refreshTube;
    int                 numOfTube;
}NixeTubeCtrl_t;

void initNixeTube(NixeTubeCtrl_t *pNixeTubeCtr);

#endif