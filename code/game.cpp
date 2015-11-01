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

#define FastAlloc(Type)  (Type *)Allocate(&GameState->Memory, sizeof(Type))
#define FastDealloc(Ptr) Deallocate(&GameState->Memory, Ptr)

DLLEXPORT void
RunFrame(gameapi_t *Api, gamestate_t *GameState)
{
    int stopmarker = 666;

    int *A = FastAlloc(int);
    *A = 0x1234;

    char *B = FastAlloc(char);
    *B = 'A';

    int *C = FastAlloc(int);
    *C = 0x5678;

    FastDealloc(B);
    FastDealloc(A);
    FastDealloc(C);
}

