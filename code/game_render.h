#ifndef GAME_RENDER_H

#include "game_memory.h"
#include "game_items.h"
#include "game_arts.h"

static gamememory_t ArtsMemory = {0};

enum renderop_e
{
    RenderOp_stop,      // Stop parsing render data stream
    RenderOp_clear,
    RenderOp_bitmap
};

struct renderdata_bitmap_t
{
    game_item_e Id;
    float X;            // [0, 1]
    float Y;            // [0, 1]
    float Width;        // 1.0 = full width
    float Height;       // 1.0 = full height
};

struct renderlist_t
{
    void *Data;
    uint At;
    uint Size;
};

static inline uint
Minimum(uint A, uint B)
{
    if (A > B) {
        return B;
    } else {
        return A;
    }
}

renderlist_t
AllocateRenderList(gamememory_t *Memory, uint Size)
{
    renderlist_t RenderList = {0};
    RenderList.Size = Size;
    RenderList.Data = Allocate(Memory, Size);
    return RenderList;
}

void 
AddToRenderList(renderlist_t *RenderList, void *Object, uint Size)
{
    assert((RenderList->Size - RenderList->At) >= Size);
    memcpy(((u8 *)RenderList->Data) + RenderList->At, Object, Size);
    RenderList->At += Size;
}

#define ADD_RENDERLIST(RenderList, Object)  AddToRenderList(RenderList, Object, sizeof(*Object))

static
void ADD_RENDERLIST_OP(renderlist_t *RenderList, renderop_e Op)
{
    renderop_e PushedOp = Op;
    ADD_RENDERLIST(RenderList, &PushedOp);
}

static uint
FloatColorToRGB(float R, float G, float B, float A = 1.0f)
{
    // ARGB
    uint ARGB = (uint)(255.0f * R) << 16 |
                (uint)(255.0f * G) << 8 |
                (uint)(255.0f * B) |
                (uint)(255.0f * A) << 24;
    return ARGB;
}

void
Clear(renderlist_t *RenderList, float R = 0, float G = 0, float B = 0)
{
    ADD_RENDERLIST_OP(RenderList, RenderOp_clear);
    uint Color = FloatColorToRGB(R, G, B);
    ADD_RENDERLIST(RenderList, &Color);
}

void
DrawBitmap(renderlist_t *RenderList, game_item_e BitmapId, float X, float Y,
           float Width = 1.0, float Height = 1.0)
{
    ADD_RENDERLIST_OP(RenderList, RenderOp_bitmap);

    renderdata_bitmap_t Bitmap;
    Bitmap.Id = BitmapId;
    Bitmap.X = X;
    Bitmap.Y = Y;
    Bitmap.Width = Width;
    Bitmap.Height = Height;
    ADD_RENDERLIST(RenderList, &Bitmap);
}

void
PerformRender(renderlist_t *RenderList, gamestate_t *GameState)
{
    drawbuffer_t *DrawBuffer = &GameState->DrawBuffer;
    uint BufferSize = DrawBuffer->Width * DrawBuffer->Height;
    u8 *Pixels  = (u8 *)(DrawBuffer->Buffer);
    u8 *Address = (u8 *)RenderList->Data;

    for (renderop_e RenderOp = (renderop_e)*Address;
         RenderOp != RenderOp_stop;
         RenderOp = (renderop_e)*Address)
    {
        // Skip past render op
        Address += sizeof(renderop_e);

#define RENDERDATA(SubType)    (renderdata_##SubType##_t *)Address; Address += sizeof(renderdata_##SubType##_t)

        switch (RenderOp) {
            case RenderOp_clear:
            {
                uint Color = *(uint *)Address;
                Address += sizeof(uint);
                uint *Dst = (uint *)Pixels;
                for (uint i=0; i < BufferSize; ++i) {
                    *Dst++ = Color;
                }
                break;
            }

            case RenderOp_bitmap:
            {
                renderdata_bitmap_t *Bitmap = RENDERDATA(bitmap);
                artfile_entry_t *Entry = GET_ENTRY((&ArtsMemory), Bitmap->Id);
                u8 *EntryData = GET_ENTRY_DATA((&ArtsMemory), Bitmap->Id);
                uint BitmapMinX = Bitmap->X * DrawBuffer->Width;
                uint BitmapMinY = Bitmap->Y * DrawBuffer->Height;
                uint BitmapMaxX = BitmapMinX + (Bitmap->Width * Entry->Dim.Width);
                uint BitmapMaxY = BitmapMinY + (Bitmap->Height * Entry->Dim.Height);
                uint EndX = BitmapMaxX; //Minimum(BitmapMaxX, DrawBuffer->Width-1);
                uint EndY = Minimum(BitmapMaxY, DrawBuffer->Height-1);
                uint *Dst = (uint *)(Pixels + 4*(BitmapMinY * DrawBuffer->Width + BitmapMinX));
                u8 *Src = EntryData;
                for (uint Y = BitmapMinY; Y < EndY; ++Y) {
                    for (uint X = BitmapMinX; X < EndX; ++X) {
                        u8 C = *EntryData++;
                        float fC = (C ? 255 / (float)C : 0);
                        *Dst++ = FloatColorToRGB(fC, fC, fC, 0);
                    }
                    Dst += (DrawBuffer->Width - Entry->Dim.Width);
                }
                break;
            }

            default:
                assert(!"Unknown render op");
        }

#undef RENDERDATA

    }
}

#define GAME_RENDER_H
#endif
