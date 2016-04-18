#ifndef GAME_ARTS_H

#include "game_basictypes.h"

#define ARTFILE_DEFAULT_FILENAME    "game_items"

#define GET_ENTRY(Mem, Offset)   ((artfile_entry_t *)(((u8*)Mem->Data) + Offset))
#define GET_ENTRY_DATA(Mem, Offset)   ((u8 *)(((u8*)Mem->Data) + Offset + sizeof(artfile_entry_t)))

enum artfile_magic_e
{
    ArtFileMagic_unknown = 0,
    ArtFileMagic_v1 = 0xAF01
};

struct artfile_header_t
{
    artfile_magic_e Magic;
    uint EntryCount;
};

enum artfile_entrytype_e
{
    ArtFile_bitmap = 0
};

struct artfile_entry_t
{
    artfile_entrytype_e Type;
    char Name[16];
    uint Size;
    union {
        struct {
            uint Width;
            uint Height;
        } Dim;
    };
    // Binary data follows directly, of Size bytes
};

#define GAME_ARTS_H
#endif
