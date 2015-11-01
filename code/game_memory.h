#ifndef GAME_MEMORY_H

#include <assert.h>
#include "game.h"

/*
 * Hashmap of free bits in memory?
 */

struct memoryprefix_t
{
    bool Taken;
    uint Size;
};

static void *
Mark(char *Address, bool Taken, uint Size)
{
    memoryprefix_t *Prefix = (memoryprefix_t *)Address;
    Prefix->Taken = Taken;
    Prefix->Size = Size;
    return (void *)(Address + sizeof(Prefix));
}

void *
Allocate(gamememory_t *GameMemory, uint Size)
{
    // TODO: check memory sizes, bounds
    
    char *Address = (char *)(GameMemory->Data);
    char *LastAddress = (Address + GameMemory->Size);
    memoryprefix_t *Prefix = 0;
    bool Found = false;
    while (!Found && Address < LastAddress) {
        Prefix = (memoryprefix_t *)Address;
        if (Prefix->Taken) {
            Address = ((char *)Address) + Prefix->Size + sizeof(memoryprefix_t);
        } else {
            // Do we fit in here?
            if (Prefix->Size == 0 || Size <= Prefix->Size) {
                Found = true;
            }
        }
    }

    void *NewSpace = 0;
    if (Found && Prefix) {
        // Split free memory slot into two; one taken and one left-over that's
        // available.
        uint LeftOver = 0;
        if (Prefix->Size > 0) {
            LeftOver = Prefix->Size - Size;
        }
        assert(LeftOver >= 0);
        NewSpace = Mark(Address, true, Size);
        if (LeftOver > 0) {
            Mark(Address + Size + sizeof(memoryprefix_t), false, LeftOver);
        }
    }
    return NewSpace;
}


void
Deallocate(gamememory_t *GameMemory, void *Memory)
{
    // TODO: defragment (compress) memory
    char *Address = (char *)Memory;
    memoryprefix_t *Prefix = (memoryprefix_t *)(Address - sizeof(memoryprefix_t));
    Prefix->Taken = false;

    // Grow current freed slot's size with any additional
    // following free slots (defragment)
    char *NextAddress = Address + Prefix->Size;
    char *LastAddress = (Address + GameMemory->Size);
    memoryprefix_t *NextPrefix = (memoryprefix_t *)NextAddress;
    while (!NextPrefix->Taken && NextPrefix->Size > 0 && NextAddress < LastAddress) {
        Prefix->Size += NextPrefix->Size;
        NextAddress += Prefix->Size;
        memoryprefix_t *NextPrefix = (memoryprefix_t *)NextAddress;
    }
}

#define GAME_MEMORY_H
#endif
