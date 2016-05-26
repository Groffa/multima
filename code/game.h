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

struct gameinput_t
{
    // TODO: mouse buttons, etc.
    bool Shift;
    bool Ctrl;
    bool Left;
    bool Right;
    bool Up;
    bool Down;
};

struct gamestate_t
{
    bool Initialized;
    bool Running;

    drawbuffer_t DrawBuffer;

    gamememory_t Memory;

    gameinput_t Input;

    uint64 DeltaTime;
};

struct gameapi_t;
typedef void (*startup_f)(gameapi_t *, gamestate_t *);
typedef void (*runframe_f)(gameapi_t *, gamestate_t *);
typedef void (*log_f)(const char *Text);
typedef uint (*mapfile_f)(const char *Filename, gamememory_t *Memory);
typedef gamememory_t (*alloc_f)(uint64 Size);

struct gameapi_t
{
    // Loading stuff
    void *Handle;

    // From game layer --> platform layer
    log_f Log;
    alloc_f AllocateMemory;

    // Functions
    startup_f StartUp;      // Called once, at startup
    runframe_f RunFrame;    // Called each frame
    mapfile_f MapFile;
};

#endif
