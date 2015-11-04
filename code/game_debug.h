#ifndef GAME_DEBUG_H

#include <intrin.h>

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
    debugtimer_t(char *Name, gamestate_t *GameState) 
        : start(__rdtsc()), name(Name), gs(GameState)
    {}

    ~debugtimer_t() {
        uint64 end = __rdtsc();
        gamestate_t *GameState = gs;
        debugmarker_t *dbg = FrameAlloc(debugmarker_t);
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

#define TIMER(Name) debugtimer_t timer##__LINE__(Name, GameState);

#define GAME_DEBUG_H
#endif
