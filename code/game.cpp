#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <memory.h>
#include "game.h"
#include "game_render.h"
#include "game_memory.h"
#include "game_debug.h"

#define DLLEXPORT   extern "C" __declspec(dllexport)

gamememory_t GlobalDebugMemory = {0};

static gamememory_t FrameMemory = {0};

DLLEXPORT void
RunFrame(gameapi_t *Api, gamestate_t *GameState)
{
    int stopmarker = 666;

    if (FrameMemory.Size == 0) {
        FrameMemory = AllocateSubGameMemory(&GameState->Memory, GameState->Memory.Size / 4);
    } else {
        // Clear frame memory each time (e.g. it works like the stack)
        memset(FrameMemory.Data, 0, FrameMemory.Size);
    }

    TIMER("Frame");
    {
        TIMER("Alloc A");
        int *A = Alloc(int, &FrameMemory);
        *A = 0x1234;
    }

    char *B = Alloc(char, &FrameMemory);
    *B = 'A';

    int *C = Alloc(int, &FrameMemory);
    *C = 0x5678;

    //char *D = Alloc(char[10], &FrameMemory);
    //*D = "Hej\0";
}

#if defined(MULTIMA_DEBUG)

DLLEXPORT void
DebugBeginFrame(gameapi_t *Api, gamestate_t *GameState)
{
    if (GlobalDebugMemory.Size == 0) {
        GlobalDebugMemory = AllocateSubGameMemory(&GameState->Memory, GameState->Memory.Size / 4);
    }
    memset(GlobalDebugMemory.Data, 0, GlobalDebugMemory.Size);
}

DLLEXPORT void
DebugEndFrame(gameapi_t *Api, gamestate_t *GameState)
{
    // TODO: collect debug data from frame
    char *Address = (char *)GlobalDebugMemory.Data;
    char *LastAddress = Address + GlobalDebugMemory.Size;
    while (Address < LastAddress) {
        memoryprefix_t *Prefix = (memoryprefix_t *)Address;
        if (Prefix->Taken) {
            debugmarker_t *dbg = (debugmarker_t *)(Address + sizeof(memoryprefix_t));
            DebugPrint(dbg, Api->Log);
        }
        Address += sizeof(memoryprefix_t) + Prefix->Size;
    }
}

#endif
