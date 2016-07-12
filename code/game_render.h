#ifndef GAME_RENDER_H

enum renderop_e
{
    RenderOp_stop,      // Stop parsing render data stream
    RenderOp_clear,
    RenderOp_bitmap,
    RenderOp_dbgfont,
};

// NOTE: Will be premultiplied alpha
struct color_t
{
    float R;
    float G;
    float B;
    float A;
};

enum bitmap_flags_e
{
    BitmapFlag_NothingSpecial = 0,
    BitmapFlag_ColorKey    = 0x1,     // Transparent? Black is always the transparent color in that case
    BitmapFlag_Alpha       = 0x2,
    BitmapFlag_AlignLeft   = 0x4,
    BitmapFlag_AlignTop    = 0x8,
    BitmapFlag_AlignRight  = 0x10,
    BitmapFlag_AlignBottom = 0x20,
};

enum game_item_e;

struct renderdata_bitmap_t
{
    game_item_e Id;
    float X;              // [0, 1]
    float Y;              // [0, 1]
    float Width;          // 1.0 = full width
    float Height;         // 1.0 = full height
    uint Color;           // Use FloatColorToRGB to compute this
    bitmap_flags_e Flags;
};

struct renderdata_debugtext_t
{
    char *Text;
    float X;            // [0, 1]
    float Y;            // [0, 1]
};

struct renderlist_t
{
    void *Data;
    uint At;
    uint Size;
};

inline color_t
Color(float R, float G, float B, float A = 1.0f)
{
    // NOTE: premultiplied alpha
    color_t Color;
    Color.R = R * A;
    Color.G = G * A;
    Color.B = B * A;
    Color.A = A;
    return Color;
}

inline color_t
White()
{
    return Color(1, 1, 1);
}

renderlist_t AllocateRenderList(gamememory_t *Memory, uint Size);

void Clear(renderlist_t *RenderList, float R = 0, float G = 0, float B = 0);
void DrawBitmap(renderlist_t *RenderList, game_item_e BitmapId, float X, float Y,
                color_t Color = White(), float Width = 1.0, float Height = 1.0,
                u8 BitmapFlags = BitmapFlag_ColorKey);
void DrawDebugText(renderlist_t *RenderList, char *Text, float X, float Y);
void PerformRender(renderlist_t *RenderList, gamestate_t *GameState);

#define GAME_RENDER_H
#endif
