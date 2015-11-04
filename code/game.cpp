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

DLLEXPORT void
RunFrame(gameapi_t *Api, gamestate_t *GameState)
{
    int stopmarker = 666;

    TIMER("Frame");

    {
        TIMER("FrameAlloc A");
        int *A = FrameAlloc(int);
        *A = 0x1234;
    }

    char *B = FrameAlloc(char);
    *B = 'A';

    int *C = FrameAlloc(int);
    *C = 0x5678;
}

