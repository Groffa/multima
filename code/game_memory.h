#ifndef GAME_MEMORY_H

#define Alloc(Memory, Type)     (Type *)Allocate(Memory, sizeof(Type)) 

enum memorytag_e
{
    MemoryTag_NotSpecified
};

struct memorylink_t
{
    bool Taken;
    uint Size;
    memorytag_e Tag;
    memorylink_t *Next;
};

void * Allocate(gamememory_t *GameMemory, uint Size, memorytag_e Tag = MemoryTag_NotSpecified);
void Deallocate(void *Ptr, bool Clear = false);
gamememory_t AllocateSubGameMemory(gamememory_t *GameMemory, uint Size);

#define GAME_MEMORY_H
#endif
