#ifndef GAME_MEMORY_H

#include "game.h"

/*
 * Hashmap of free bits in memory?
 */

void *
Allocate(gamememory_t *GameMemory, uint64 Size)
{
    // TODO: check memory sizes, bounds
    return malloc(Size); 
}


void
Deallocate(gamememory_t *GameMemory, void *Memory)
{
    // TODO: defragment (compress) memory
    
    // Example:
    // 1. Memory layout before deallocate(C) (. is free)
    //    AAABBCCCCDFFFF...
    //                  ^
    //                   \____ Next free
    //
    // 2. After call to deallocate(C):
    //    AAABB....DFFFF...
    //                  ^
    //                   \____ Next free
    //
    // 3. Compress (move) rest of memory backwards:
    //    move(C->Data + C->Size, C->Data, TotalMemory - C->Data)
    //    AAABBDFFFF.......
    //
    // 4. Adjust next free-pointer
    //    NextFree->Data -= C->Size
    //    NextFree->Size += C->Size
    //
    // Special case:
    // When C->Data + C->Size == NextFree, then just move back next free

    free(Memory);
}

#define GAME_MEMORY_H
#endif
