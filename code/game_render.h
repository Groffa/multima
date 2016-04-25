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

struct color_t
{
    float R;
    float G;
    float B;
    float A;
};

struct renderdata_bitmap_t
{
    game_item_e Id;
    float X;            // [0, 1]
    float Y;            // [0, 1]
    float Width;        // 1.0 = full width
    float Height;       // 1.0 = full height
    uint Color;         // Use FloatColorToRGB to compute this
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

inline color_t
Color(float R, float G, float B, float A = 1.0f)
{
    return { R, G, B, A };
}

inline color_t
White()
{
    return Color(1, 1, 1);
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
           color_t Color = White(), float Width = 1.0, float Height = 1.0)
{
    ADD_RENDERLIST_OP(RenderList, RenderOp_bitmap);

    renderdata_bitmap_t Bitmap;
    Bitmap.Id = BitmapId;
    Bitmap.X = X;
    Bitmap.Y = Y;
    Bitmap.Width = Width;
    Bitmap.Height = Height;
    Bitmap.Color = FloatColorToRGB(Color.R, Color.G, Color.B, Color.A);
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
                int StartX = Bitmap->X * DrawBuffer->Width;
                int StartY = Bitmap->Y * DrawBuffer->Height;
               
                if (((StartX + Entry->Dim.Width < 0) && (StartY + Entry->Dim.Height < 0)) ||
                     (StartX > DrawBuffer->Width) && (StartY > DrawBuffer->Height))
                {
                    // Not visible
                    break;
                }

                uint RealDimWidth = Entry->Dim.Width;
                uint RealDimHeight = Entry->Dim.Height;

                u8 *Src = EntryData;
                if (StartY < 0) {
                    Src += abs(StartY) * Entry->Dim.Width;
                    RealDimHeight -= abs(StartY);
                    StartY = 0;
                }
                if (StartX < 0) {
                    Src += abs(StartX);
                    RealDimWidth -= abs(StartX);
                    StartX = 0;
                }

                if (StartX + RealDimWidth > DrawBuffer->Width) {
                    RealDimWidth -= (StartX + RealDimWidth) - DrawBuffer->Width;
                }
                if (StartY + RealDimHeight > DrawBuffer->Height) {
                    RealDimHeight -= (StartY + RealDimHeight) - DrawBuffer->Height;
                }

                uint *Dst = (uint *)(Pixels + 4 * (StartY * DrawBuffer->Width + StartX));

                for (uint Y = 0; Y < Entry->Dim.Height; ++Y) {
                    if (Y >= RealDimHeight) {
                        break;
                    }
                    for (uint X = 0; X < Entry->Dim.Width; ++X) {
                        u8 C = *Src++;
                        uint FinalColor = 0;
                        if (C) {
                            FinalColor = Bitmap->Color;
                        }
                        if (X < RealDimWidth) {
                            *Dst = FinalColor;
                        }
                        ++Dst;
                    }
                    Dst += DrawBuffer->Width - Entry->Dim.Width;
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
