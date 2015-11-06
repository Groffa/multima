#ifndef GAME_DEBUG_H

#include <intrin.h>
#include "game.h"

extern "C" gamememory_t GlobalDebugMemory;

enum debugtype_e
{
    DebugType_Timer
};

struct debugmarker_t
{
    debugtype_e Type;
    char Name[32];
    union {
        uint64 UInt64;
        bool Bool;
    };
};

struct debugtimer_t
{
    debugtimer_t(char *Name) 
        : start(__rdtsc()), name(Name)
    {}

    ~debugtimer_t() {
        uint64 end = __rdtsc();
        assert(GlobalDebugMemory.Size > 0);
        assert(GlobalDebugMemory.Data != 0);
        debugmarker_t *dbg = Alloc(debugmarker_t, &GlobalDebugMemory);
        char *p = name;
        uint i = 0;
        while (*p) {
            dbg->Name[i++] = *p++;
        }
        dbg->Name[i] = 0;
        dbg->Type = DebugType_Timer;
        dbg->UInt64 = end - start;
    }

    char *name;
    uint64 start;
    gamestate_t *gs;
};

#define TIMER(Name) debugtimer_t timer##__LINE__(Name);

#define GAME_DEBUG_H
#endif
