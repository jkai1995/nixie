#include "tubeDriver.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "74HC595.h"

// num    1    2    4    8    16    32    64    128
//buf[0]  7    6    5    4     3    2      1     0      四
//buf[1]                                   9     8
//buf[1]  5    4    3    2     1    0                   三
//buf[2]                       9    8      7     6
//buf[2]  3    2    1    0                              二
//buf[3]            9    8     7    6      5     4
//buf[3]  1    0                                        一
//buf[4]  9    8    7    6     5    4      3     2

#define NUM_OF_TUBE 4  //we have 4 nixie tube
#define NUM_OF_NUMBER 10 //we have 10 numbers(0-9) to show per tube 
#define NUM_OF_DOT 2

typedef struct
{
    u8 idx;//buffer index of 74HC595 array
    u8 bit;//bit of per buffer
}st_BufferOf74HC595;

static st_BufferOf74HC595 m_tubeBuffer[NUM_OF_TUBE][NUM_OF_NUMBER] = 
{
//tube 0
//     0     1     2     3     4     5     6     7     8     9
    {{3,1},{3,0},{4,7},{4,6},{4,5},{4,4},{4,3},{4,2},{4,1},{4,0}},
//tube 1
//       0     1       2     3       4     5       6     7       8     9
    {{2,3},{2,2},{2,1},{2,0},{3,7},{3,6},{3,5},{3,4},{3,3},{3,2}},
//tube 2
//       0     1       2     3       4     5       6     7       8     9
    {{1,5},{1,4},{1,3},{1,2},{1,1},{1,0},{2,7},{2,6},{2,5},{2,4}},
//tube 3
//       0     1       2     3       4     5       6     7       8     9
    {{0,7},{0,6},{0,5},{0,4},{0,3},{0,2},{0,1},{0,0},{1,7},{1,6}}
};

static st_BufferOf74HC595 m_dotBuffer[NUM_OF_DOT] =
{
    {5,6},{5,7}
};

static int clearTube(u32 idx)
{
    for (int i = 0; i < NUM_OF_NUMBER; i++)
    {
        clearBit74HC595(m_tubeBuffer[idx][i].idx,m_tubeBuffer[idx][i].bit);
    }
    return 0;
}

static int setTube(u32 idx, s8 value)
{
    if (idx >= 0 && idx < NUM_OF_TUBE)
    {
        if (value >= 0 && value < NUM_OF_NUMBER)
        {
            clearTube(idx);
            return setBit74HC595(m_tubeBuffer[idx][value].idx,m_tubeBuffer[idx][value].bit);
        }
        else if (value == -1)
        {
            return clearTube(idx);
        }
    }
    return -1;
}

static int setDote()
{
    for (int i = 0; i < NUM_OF_DOT; i++)
    {
        setBit74HC595(m_dotBuffer[i].idx,m_dotBuffer[i].bit);
    }
    return 0;
}

static int clearDote()
{
    for (int i = 0; i < NUM_OF_DOT; i++)
    {
        clearBit74HC595(m_dotBuffer[i].idx,m_dotBuffer[i].bit);
    }
    return 0;
}

void initNixeTube(NixeTubeCtrl_t *pNixeTubeCtr)
{
    pNixeTubeCtr->clearTube = clearTube;
    pNixeTubeCtr->setTube = setTube;
    pNixeTubeCtr->setDote = setDote;
    pNixeTubeCtr->clearDote = clearDote;
    pNixeTubeCtr->numOfTube = NUM_OF_TUBE;
    pNixeTubeCtr->refreshTube = writeRegTo74HC595;

    init74HC595();
}


