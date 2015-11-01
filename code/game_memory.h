#ifndef GAME_MEMORY_H

#include "game.h"

/*
 * Hashmap of free bits in memory?
 */

struct memoryprefix_t
{
    bool Taken;
    uint Size;
};

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
            Found = true;
        }
    }

    void *NewSpace = 0;
    if (Found && Prefix) {
        Prefix->Taken = true;
        Prefix->Size = Size;
        NewSpace = (void *)(Address + sizeof(memoryprefix_t));
    }
    return NewSpace;
}


void
Deallocate(gamememory_t *GameMemory, void *Memory)
{
    // TODO: defragment (compress) memory
    
    memoryprefix_t *Prefix = (memoryprefix_t *)(((char *)Memory) - sizeof(memoryprefix_t));
    Prefix->Taken = false;
}

#define GAME_MEMORY_H
#endif
