#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <memory.h>
#include "game.h"
#include "game_arts.h"
#include "game_render.h"
#include "game_memory.h"

// For testing
#include "game_items.h"

#define DLLEXPORT   extern "C" __declspec(dllexport)

static gamememory_t FrameMemory = {0};
extern gamememory_t ArtsMemory;     // in game_render.h
extern gamememory_t DebugMemory;    // in game_debug.h

static void
LoadArts(const char *Filename, gameapi_t *Api, gamememory_t *Memory)
{
    Api->MapFile(Filename, Memory);
    artfile_header_t *Header = (artfile_header_t *)Memory->Data;

    if (Header->Magic != ArtFileMagic_v1) {
        assert(!"Wrong art magic");
    }
}

DLLEXPORT void
StartUp(gameapi_t *Api, gamestate_t *GameState)
{
    GameState->Memory = Api->AllocateMemory(MEGABYTES(32)); 
    FrameMemory = AllocateSubGameMemory(&GameState->Memory, MEGABYTES(10));
    ArtsMemory = AllocateSubGameMemory(&GameState->Memory, MEGABYTES(10));

    LoadArts("game_items.art", Api, &ArtsMemory);

    GameState->Initialized = true;
}

static float X = 1;
static float Y = 1;

DLLEXPORT void
RunFrame(gameapi_t *Api, gamestate_t *GameState)
{
    // Clear frame memory each time (e.g. it works like the stack)
    memset(FrameMemory.Data, 0, FrameMemory.Size);

    /*
    // NOTE: keep this code in this comment for now in case I forget
    // how to use this system!
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
    DrawBitmap(&RenderList, GameItem_ruta, 0.8, 0.1f, White(), 0.5f, 1);
    DrawBitmap(&RenderList, GameItem_demon, 0, 0, Color(1,0,0,0.5f), 1, 1, BitmapFlag_Alpha);
    DrawBitmap(&RenderList, GameItem_demon, 0, 0, Color(0,0,1,0.5f), 1, 1, BitmapFlag_AlignBottom|BitmapFlag_Alpha);
    DrawBitmap(&RenderList, GameItem_demon, 0, 0, Color(1,0,1,0.75f), 1.5, 1, BitmapFlag_AlignRight|BitmapFlag_Alpha);
    DrawBitmap(&RenderList, GameItem_demon, 0.15f, 0.15f, Color(0,1,0,0.25f), 1, 1, Flags);
    PerformRender(&RenderList, GameState);
    */

    if (GameState->Input.Right) {
        X += 0.01;
    } else if (GameState->Input.Left) {
        X -= 0.01;
    }
    if (GameState->Input.Up) {
        Y -= 0.01;
    } else if (GameState->Input.Down) {
        Y += 0.01;
    }

    const float CharWidth = X * (8 / (float)GameState->DrawBuffer.Width);
    float cX = 0;
#define DBGPRINT(c,x) DrawBitmap(&RenderList, GameItem_debugfont_##c, cX, 0, White(), X, Y); cX += CharWidth; 
    renderlist_t RenderList = AllocateRenderList(&FrameMemory, MEGABYTES(1));
    Clear(&RenderList);
    DBGPRINT(a,0);
    DBGPRINT(b,0.1);
    DBGPRINT(c,0.15);
    DBGPRINT(d,0.2);
    DBGPRINT(e,0.25);
    DBGPRINT(f,0.3);
    PerformRender(&RenderList, GameState);
}

#include "game_render.cpp"
#include "game_memory.cpp"
