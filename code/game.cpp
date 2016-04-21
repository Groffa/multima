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
extern gamememory_t ArtsMemory;     // in game_render.h

static void
InitSubMemories(gamestate_t *GameState)
{
    if (FrameMemory.Size == 0) {
        FrameMemory = AllocateSubGameMemory(&GameState->Memory, MEGABYTES(10));
    }
    if (ArtsMemory.Size == 0) {
        ArtsMemory = AllocateSubGameMemory(&GameState->Memory, MEGABYTES(10));
    }
}

#include "game_items.h"

static void
LoadArts(const char *Filename, gameapi_t *Api, gamememory_t *Memory)
{
    Api->MapFile(Filename, Memory);
    artfile_header_t *Header = (artfile_header_t *)Memory->Data;

    if (Header->Magic != ArtFileMagic_v1) {
        assert(!"Wrong art magic");
    }
#if 0
    artfile_entry_t *Entries[] = {
        {GET_ENTRY(Memory, GameItem_abc80ways2die)},
        {GET_ENTRY(Memory, GameItem_kungfu)}
    };
    u8 *kungfu_bitmap = GET_ENTRY_DATA(Memory, GameItem_kungfu);
    uint hej=123;
#endif
}

static float X = 0.7f; static float dX = 0.002f;
static float Y = 0; static float dY = 0.001f;

DLLEXPORT void
RunFrame(gameapi_t *Api, gamestate_t *GameState)
{
    int stopmarker = 666;

    if (!GameState->Initialized) {
        InitSubMemories(GameState);
        LoadArts("game_items.art", Api, &ArtsMemory);
        GameState->Initialized = true;
    }

    /*
    uint PageSize = 4096;
    char *OnePage = (char *)Allocate(&GameState->Memory, 4096 - sizeof(memorylink_t));;
    //char *OnePage = (char *)Allocate(&GameState->Memory, 4096);
    for (uint i=0; i < PageSize - sizeof(memorylink_t); ++i) {
        *(OnePage + i) = 'A' + (i % 26);
    }
    Deallocate(OnePage, true);
    */

    // Clear frame memory each time (e.g. it works like the stack)
    memset(FrameMemory.Data, 0, FrameMemory.Size);

    if (GameState->Input.Right) {
        X += (100/64000.0f);
    } else if (GameState->Input.Left) {
        X -= (100/64000.0f);
    }
    if (GameState->Input.Up) {
        Y -= (100/48000.0f);
    } else if (GameState->Input.Down) {
        Y += (100/48000.0f);
    }

#if 0
    X += dX;
    Y += dY;
    //if (X < abs((int)dX) || X > 1-(100/640.0f)) { dX = -dX; }
    if (Y < abs((int)dY) || Y > 1-(100/480.0f)) { dY = -dY; }
#endif

    renderlist_t RenderList = AllocateRenderList(&FrameMemory, MEGABYTES(1));
    Clear(&RenderList);
    DrawBitmap(&RenderList, GameItem_demon, X, Y);
    PerformRender(&RenderList, GameState);
}
