#include "game_items.h"
#include "game_memory.h"
#include "game_arts.h"
#include "game_render.h"

static gamememory_t ArtsMemory = {0};

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

static color_t
UintToColor(uint Value)
{
    // AARRGGBB
    float Inv255 = 1 / 255.0f;
    color_t Color = {0};
    Color.R = (Value & 0x00FF0000) >> 16;
    Color.G = (Value & 0x0000FF00) >> 8;
    Color.B = Value & 0xFF;
    Color.A = Value >> 24;

    Color.R *= Inv255;
    Color.G *= Inv255;
    Color.B *= Inv255;
    Color.A *= Inv255;

    return Color;
}

static uint
ColorToRGB(color_t Color)
{
    return FloatColorToRGB(Color.R, Color.G, Color.B, Color.A);
}

void
Clear(renderlist_t *RenderList, float R, float G, float B)
{
    ADD_RENDERLIST_OP(RenderList, RenderOp_clear);
    uint Color = FloatColorToRGB(R, G, B);
    ADD_RENDERLIST(RenderList, &Color);
}

void
DrawBitmap(renderlist_t *RenderList, game_item_e BitmapId, float X, float Y,
           color_t Color, float Width, float Height,
           u8 BitmapFlags)
{
    ADD_RENDERLIST_OP(RenderList, RenderOp_bitmap);

    renderdata_bitmap_t Bitmap;
    Bitmap.Id = BitmapId;
    Bitmap.X = X;
    Bitmap.Y = Y;
    Bitmap.Width = Width;
    Bitmap.Height = Height;
    Bitmap.Color = ColorToRGB(Color);
    Bitmap.Flags = (bitmap_flags_e)BitmapFlags;
    ADD_RENDERLIST(RenderList, &Bitmap);
}

void
DrawDebugText(renderlist_t *RenderList, char *Text, float X = 0, float Y = 0)
{
    ADD_RENDERLIST_OP(RenderList, RenderOp_dbgfont);
    /*
    renderdata_debugtext_t DebugText;
    DebugText.Text = Text;
    DebugText.X = X;
    DebugText.Y = Y;

    ADD_RENDERLIST(RenderList, &DebugText);
    */

    // TODO: render each letter as bitmap and add them to ArtsMemory,
    // so that we may use DrawBitmap instead (e.g. this function only
    // initializes the debug font)
}

static bool
BoundsCheck(void *Base, uint Size, void *Ptr)
{
    u8 *Start = (u8 *)Base;
    u8 *End = Start + Size;
    u8 *iPtr = (u8 *)Ptr;
    return iPtr >= Start && iPtr <= End;
}

void
PerformRender(renderlist_t *RenderList, gamestate_t *GameState)
{
    drawbuffer_t *DrawBuffer = &GameState->DrawBuffer;
    uint BufferSize = DrawBuffer->Width * DrawBuffer->Height;
    u8 *Pixels  = (u8 *)(DrawBuffer->Buffer);
    u8 *Address = (u8 *)RenderList->Data;
    
    bool FirstRender = false;

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
                FirstRender = true; // Why? Because everything's gone now
                break;
            }

            case RenderOp_bitmap:
            {
                renderdata_bitmap_t *Bitmap = RENDERDATA(bitmap);
                artfile_entry_t *Entry = GET_ENTRY((&ArtsMemory), Bitmap->Id);
                u8 *EntryData = GET_ENTRY_DATA((&ArtsMemory), Bitmap->Id);
                int StartX = Bitmap->X * DrawBuffer->Width;
                int StartY = Bitmap->Y * DrawBuffer->Height;
                bool UseColorKey = Bitmap->Flags & BitmapFlag_ColorKey; 
                bool UseAlpha = Bitmap->Flags & BitmapFlag_Alpha;

                if (Bitmap->Flags & BitmapFlag_AlignLeft) {
                    StartX = 0;
                } else if (Bitmap->Flags & BitmapFlag_AlignRight) {
                    StartX = DrawBuffer->Width - Entry->Dim.Width * Bitmap->Width;
                }
                if (Bitmap->Flags & BitmapFlag_AlignTop) {
                    StartY = 0;
                } else if (Bitmap->Flags & BitmapFlag_AlignBottom) {
                    StartY = DrawBuffer->Height - Entry->Dim.Height * Bitmap->Height;
                }

                /*
                if (((StartX + Entry->Dim.Width < 0) && (StartY + Entry->Dim.Height < 0)) ||
                     (StartX > DrawBuffer->Width) && (StartY > DrawBuffer->Height))
                {
                    // Not visible
                    break;
                }
                */

                Bitmap->Width = (Bitmap->Width < 0 ? 0 : Bitmap->Width);
                Bitmap->Height = (Bitmap->Height < 0 ? 0 : Bitmap->Height);

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
                // uint *LastDst = (uint *)(Pixels + 4 * DrawBuffer->Width * DrawBuffer->Height - 1);
                float StepX = 1 / Bitmap->Width;
                float StepY = 1 / Bitmap->Height;

                uint DrawBufferSize = 4 * DrawBuffer->Width * DrawBuffer->Height - 1;

                for (float Y = 0; Y < Entry->Dim.Height; Y += StepY) {
                    if (((uint)(Y+0.5f)) >= RealDimHeight || !BoundsCheck(Pixels, DrawBufferSize, Dst) /*Dst > LastDst*/) {
                        break;
                    }
                    uint iY = (uint)Y;
                    uint Scanline = 0;
                    for (float X = 0; X < Entry->Dim.Width; X += StepX) {
                        //if (Dst > LastDst) {
                        if (Scanline >= DrawBuffer->Width ||
                            !BoundsCheck(Pixels, DrawBufferSize, Dst))
                        {
                            break;
                        }
                        uint iX = (uint)X;
                        u8 C = *(Src + iY * Entry->Dim.Width + iX);
                        uint FinalColor = 0; 
                        if (C) {
                            FinalColor = Bitmap->Color;
                        }
                        if (X < RealDimWidth &&
                            (!UseColorKey || FinalColor))
                        {
                            if (!FirstRender && UseAlpha) {
                                uint DstRaw = *Dst;
                                color_t DstColor = UintToColor(DstRaw);
                                color_t SrcColor = UintToColor(FinalColor);
                                float OneMinusSrcA = 1.0f - SrcColor.A;
                                DstColor.R = SrcColor.R + (DstColor.R * OneMinusSrcA);
                                DstColor.G = SrcColor.G + (DstColor.G * OneMinusSrcA);
                                DstColor.B = SrcColor.B + (DstColor.B * OneMinusSrcA);
                                FinalColor = ColorToRGB(DstColor);
                            }
                            *Dst = FinalColor;
                        }
                        ++Dst;
                        ++Scanline;
                    }
                    Dst += DrawBuffer->Width - Scanline; 
                }
                FirstRender = false;
                break;
            }

            default:
                assert(!"Unknown render op");
        }

#undef RENDERDATA

    }
}

