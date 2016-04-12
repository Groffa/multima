#ifndef GAME_MEMORY2_H

#include <assert.h>
#include "game.h"

#define Alloc(Memory, Type)     (Type *)Allocate(Memory, sizeof(Type)) 

struct memorylink_t
{
    bool Taken;
    uint Size;
    memorylink_t *Next;
};

static inline int
RemainingSpace(gamememory_t *GameMemory)
{
    uint64 SpaceLeft = GameMemory->Size;
    memorylink_t *Link = (memorylink_t *)GameMemory->Data;
    uint64 *LastAddress = (uint64 *)(((char *)GameMemory->Data) + GameMemory->Size);
    while (((uint64 *)Link) < LastAddress && Link->Next && Link->Taken) {
        SpaceLeft -= (Link->Size + sizeof(memorylink_t));
        Link = Link->Next;
    }
    return SpaceLeft;
}

static memorylink_t *
FindFirstFit(gamememory_t *GameMemory, uint Size)
{
    uint64 SpaceLeft = GameMemory->Size;
    memorylink_t *Link = (memorylink_t *)GameMemory->Data;
    if (!Link->Taken) {
        // First-time initializing
        Link->Size = GameMemory->Size - sizeof(memorylink_t);
    }
    uint64 *LastAddress = (uint64 *)(((char *)GameMemory->Data) + GameMemory->Size);
    do {
        if (!Link->Taken && (Link->Size == 0 || Link->Size >= Size)) {
            break;
        }
        SpaceLeft -= (Link->Size + sizeof(memorylink_t));
        if (SpaceLeft < Size) {
            Link = 0;
            break;
        }
        Link = Link->Next;
    } while (Link && (uint64 *)Link < LastAddress);
    return Link;
}

void *
Allocate(gamememory_t *GameMemory, uint Size)
{
    memorylink_t *Link = FindFirstFit(GameMemory, Size);
    // TODO: check if Link is valid (otherwise we may be out of memory)
    assert(Link);

    Link->Taken = true;
    Link->Size = Size;
    Link->Next = (memorylink_t *)(((char *)Link) + sizeof(memorylink_t) + Size);
    return (void *)(((char *)Link) + sizeof(memorylink_t));
}

void
Deallocate(void *Ptr, bool Clear = false)
{
    memorylink_t *Link = (memorylink_t *)(((char *)Ptr) - sizeof(memorylink_t));
    Link->Taken = false;
    if (Clear) {
        memset(Ptr, 0, Link->Size);
    }
}

#define GAME_MEMORY2_H
#endif
