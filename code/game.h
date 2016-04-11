#ifndef GAME_H
#define GAME_H

#include "game_basictypes.h"

struct drawbuffer_t
{
    uint Width;
    uint Height;
    void *Buffer;
};

struct gamememory_t
{
    void *Data;
    uint64 Size;
};

struct gamestate_t
{
    bool Initialized;
    bool Running;

    drawbuffer_t DrawBuffer;

    gamememory_t Memory;

    uint64 DeltaTime;
};

struct gameapi_t;
typedef void (*runframe_f)(gameapi_t *, gamestate_t *);
typedef void (*log_f)(const char *Text);
typedef uint (*mapfile_f)(const char *Filename, gamememory_t *Memory);

struct gameapi_t
{
    // Loading stuff
    void *Handle;

    // From game.dll --> platform.cpp
    log_f Log;

    // Functions
    runframe_f RunFrame;
    mapfile_f MapFile;
};

#endif
