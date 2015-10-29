#ifndef GAME_H
#define GAME_H

struct gameapi_t;
struct gamestate_t;

typedef void (*RunFrameFunc)(gameapi_t *, gamestate_t *);
typedef void (*LogFunc)(const char *Text);

typedef void (*TestFunc)(void *t); 

typedef unsigned int uint;
typedef unsigned __int64 uint64;

struct drawbuffer_t
{
    uint Width;
    uint Height;
    void *Buffer;
};

struct memory_t
{
    void *Data;
    uint64 Size;
};

struct gamememory_t
{
    memory_t Total;
    memory_t NextFree;
};

struct gamestate_t
{
    bool Initialized;
    bool Running;

    drawbuffer_t DrawBuffer;

    gamememory_t Memory;

    uint64 DeltaTime;
};

struct gameapi_t
{
    // Loading stuff
    void *Handle;

    // From game.dll --> platform.cpp
    LogFunc Log;

    // Functions
    RunFrameFunc RunFrame;
};

#endif
