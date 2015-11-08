#ifndef GAME_RENDER_H

#include "game_memory.h"

/*
 * Game render pipeline:
 * (http://www.cs.princeton.edu/courses/archive/fall99/cs426/lectures/pipeline/sld007.htm)
 * 1. Modeling transformation
 *    Transform geometric primitives into WORLD coordinate
 *    system.
 * 2. Clipping
 *    Clip portions of geometric primitves residing ouside
 *    the window.
 * 3. Viewing transformation
 *    Transform the clipped primitives from WORLD to SCREEN
 *    coordinates.
 * 4. Scan conversion
 *    Fill pixels representing primitives
 *
 */

enum renderop_e
{
    RenderOp_Invalid,
    RenderOp_SetColor,
    RenderOp_Point
};

struct rendercolor_t
{
    float R;
    float G;
    float B;
    float A;
};

struct renderposition_t
{
    float X;
    float Y;
};

struct renderdata_t
{
    renderop_e Op;
    union {
        rendercolor_t Color;
        renderposition_t Position;
    };
};

struct renderstate_t
{
    rendercolor_t Color;
    renderposition_t Position;
};

static renderstate_t RenderState = {0};

static uint
FloatColorToRGB(rendercolor_t Color)
{
    // ARGB
    uint ARGB = 0 |
                (int)(255.0f * Color.R) << 16 |
                (int)(255.0f * Color.G) << 8 |
                (int)(255.0f * Color.B) |
                (int)(255.0f * Color.A) << 24;
    return ARGB;
}

void
RenderPoint(gamememory_t *GameMemory, float X, float Y, float R, float G, float B)
{
    renderdata_t *RenderData = Alloc(renderdata_t, GameMemory);
    RenderData->Op = RenderOp_SetColor;
    RenderData->Color = { R, G, B, 1.0f };

    RenderData = Alloc(renderdata_t, GameMemory);
    RenderData->Op = RenderOp_Point;
    RenderData->Position = { X, Y };
}

/*
 * NOTE:
 * this is only testing code. Positions are currently in world space already,
 * which is wrong since they'd be in model space naturally, so we'd need
 * to convert from model to world space.
 * And to projection after that?
 */

void
PerformRender(gamememory_t *GameMemory, gamestate_t *GameState)
{
    drawbuffer_t *DrawBuffer = &GameState->DrawBuffer;
    uint *Pixels  = (uint *)(DrawBuffer->Buffer);
    char *Address = (char *)GameMemory->Data;
    char *LastAddress = Address + GameMemory->Size;
    while (Address < LastAddress) {
        memoryprefix_t *Prefix = (memoryprefix_t *)Address;
        if (Prefix->Type == MemoryType_renderdata_t) {
            renderdata_t *RenderData = (renderdata_t *)(Address + sizeof(memoryprefix_t));
            switch (RenderData->Op) {
                case RenderOp_SetColor:
                    RenderState.Color = RenderData->Color;
                    break;

                case RenderOp_Point:
                    {
                        // NOTE: assumes world space coordinates
                        uint wX = RenderData->Position.X * (float)(DrawBuffer->Width-1);
                        uint wY = RenderData->Position.Y * (float)(DrawBuffer->Height-1);
                        *(Pixels + wY*DrawBuffer->Width + wX) = 
                            FloatColorToRGB(RenderState.Color);
                    }
                    break;

                default:
                    assert(!"Unknown render operation");
            }
        }
        Address += sizeof(memoryprefix_t) + Prefix->Size;
    }
}

#define GAME_RENDER_H
#endif
