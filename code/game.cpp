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

    Test_1 *NewData = (Test_1 *)Allocate(&GameState->Memory, sizeof(Test_1));
    NewData->A = 0xAB;
    NewData->B = 0xCD;

    for (int i=0; i < 10; ++i) {
        Allocate(&GameState->Memory, 1);
    }
    
    int blaj=123;

    Deallocate(&GameState->Memory, NewData);
}

