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
// For testing
#include "game_items.h"

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


static void
LoadArts(const char *Filename, gameapi_t *Api, gamememory_t *Memory)
{
    Api->MapFile(Filename, Memory);
    artfile_header_t *Header = (artfile_header_t *)Memory->Data;

    if (Header->Magic != ArtFileMagic_v1) {
        assert(!"Wrong art magic");
    }
}

static float X = 1;
static float Y = 1;

DLLEXPORT void
RunFrame(gameapi_t *Api, gamestate_t *GameState)
{
    int stopmarker = 666;

    if (!GameState->Initialized) {
        InitSubMemories(GameState);
        LoadArts("game_items.art", Api, &ArtsMemory);
        GameState->Initialized = true;
    }

    // Clear frame memory each time (e.g. it works like the stack)
    memset(FrameMemory.Data, 0, FrameMemory.Size);

    u8 Flags = BitmapFlag_Alpha;
    
    if (GameState->Input.Right) {
        Flags |= BitmapFlag_AlignRight; 
    } else if (GameState->Input.Left) {
        Flags |= BitmapFlag_AlignLeft;
    }
    if (GameState->Input.Up) {
        Flags |= BitmapFlag_AlignTop;
    } else if (GameState->Input.Down) {
        Flags |= BitmapFlag_AlignBottom;
    }

    renderlist_t RenderList = AllocateRenderList(&FrameMemory, MEGABYTES(1));
    Clear(&RenderList);
    DrawBitmap(&RenderList, GameItem_ruta, 0.1f, 0.1f, White(), 1, 1, Flags);
    DrawBitmap(&RenderList, GameItem_demon, 0, 0, Color(1,0,0,0.5f), 1, 1);
    DrawBitmap(&RenderList, GameItem_demon, 0.15f, 0.15f, Color(0,1,0,0.5f), 1, 1);
    PerformRender(&RenderList, GameState);
}
