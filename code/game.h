#ifndef GAME_H
#define GAME_H

struct gameapi_t;
struct gamestate_t;

typedef void (*RunFrameFunc)(gameapi_t *, gamestate_t *);
typedef void (*LogFunc)(const char *Text);

typedef unsigned int uint;
typedef unsigned __int64 uint64;

struct drawbuffer_t
{
    uint Width;
    uint Height;
    void *Buffer;
};

enum memorytype_e
{
    MemoryType_NOOP,
    MemoryType_int,
    MemoryType_debugmarker_t,
    MemoryType_char,

    MemoryType_Count
};

struct memoryprefix_t
{
    bool Taken;
    uint Size;
    memorytype_e Type;
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

struct gameapi_t
{
    // Loading stuff
    void *Handle;

    // From game.dll --> platform.cpp
    LogFunc Log;

    // Functions
    RunFrameFunc RunFrame;

#if defined(MULTIMA_DEBUG)
    RunFrameFunc DebugBeginFrame;
    RunFrameFunc DebugEndFrame;
#endif
};

#endif
