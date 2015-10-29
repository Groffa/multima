#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <memory.h>
#include "game.h"
#include "game_render.h"
#include "game_memory.h"

#define DLLEXPORT   extern "C" __declspec(dllexport)

struct Test_1
{
    int A;
    int B;
};

struct Test_2
{
    char Name[32];
    int C;
};

DLLEXPORT void
RunFrame(gameapi_t *Api, gamestate_t *GameState)
{
    int stopmarker = 666;

    void *EmptySpace = malloc(32);
    Test_1 *t1 = (Test_1*)EmptySpace;
    t1->A = 123;
    t1->B = 456;
    free(EmptySpace);

    
}

