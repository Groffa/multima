#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <memory.h>
#include "game.h"
#include "game_render.h"
#include "game_memory.h"
#include "game_arts.h"

#define DLLEXPORT   extern "C" __declspec(dllexport)

static gamememory_t FrameMemory = {0};
static gamememory_t ArtsMemory = {0};

static void
InitSubMemories(gamestate_t *GameState)
{
    if (FrameMemory.Size == 0) {
        FrameMemory = AllocateSubGameMemory(&GameState->Memory, MEGABYTES(1));
    }
    if (ArtsMemory.Size == 0) {
        ArtsMemory = AllocateSubGameMemory(&GameState->Memory, MEGABYTES(2));
    }
}

void
LoadArts(const char *Filename, gameapi_t *Api, gamememory_t *Memory)
{
    Api->MapFile(Filename, Memory);
    artfile_header_t *Header = (artfile_header_t *)Memory->Data;

    if (Header->Magic != ArtFileMagic_v1) {
        assert(!"Wrong art magic");
    }
        
    for (uint i=0; i < Header->EntryCount; ++i) {
        uint stophere = 123;
    }
}

DLLEXPORT void
RunFrame(gameapi_t *Api, gamestate_t *GameState)
{
    int stopmarker = 666;

    if (!GameState->Initialized) {
        InitSubMemories(GameState);
        //LoadArts("items.art", Api, &ArtsMemory);
        GameState->Initialized = true;
    }

    uint PageSize = 4096;
    char *OnePage = (char *)Allocate(&GameState->Memory, 4096 - sizeof(memorylink_t));;
    //char *OnePage = (char *)Allocate(&GameState->Memory, 4096);
    for (uint i=0; i < PageSize - sizeof(memorylink_t); ++i) {
        *(OnePage + i) = 'A' + (i % 26);
    }
    Deallocate(OnePage, true);

    // Clear frame memory each time (e.g. it works like the stack)
    // memset(FrameMemory.Data, 0, FrameMemory.Size);

    //void *RenderMemory = Allocate(&GameState->Memory, MEGABYTES(1));
    //RenderTexture(&RenderMemory, Position(0.25f, 0.5f), 666);
    //PerformRender(&RenderMemory, GameState);
}
