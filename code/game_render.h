#ifndef GAME_RENDER_H

#include "game_memory.h"

enum renderop_e
{
    RenderOp_invalid,
    RenderOp_texture
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

struct renderdata_header_t
{
    renderop_e Op;
};

struct renderdata_texture_t
{
    uint TextureId;
    renderposition_t Position;
};

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

renderposition_t
Position(float X, float Y)
{
    renderposition_t Position = { X, Y };
    return Position;
}

/*
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
*/

static void
BeginRenderOp(renderop_e Op, gamememory_t *Memory)
{
    renderdata_header_t *Header = Alloc(Memory, renderdata_header_t);
    Header->Op = Op;
}

void
RenderTexture(gamememory_t *Memory, renderposition_t Position, uint TextureId)
{
    BeginRenderOp(RenderOp_texture, Memory);
    renderdata_texture_t *RenderData = Alloc(Memory, renderdata_texture_t);
    RenderData->TextureId = TextureId;
    RenderData->Position = Position;
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
    char *Pixels  = (char *)(DrawBuffer->Buffer);
    unsigned char *Address = (unsigned char *)GameMemory->Data;
    unsigned char *LastAddress = Address + GameMemory->Size;

    // TODO: remove need for Address < LastAddress, just eat bytes and interpret
    // like a CPU
}

#define GAME_RENDER_H
#endif
