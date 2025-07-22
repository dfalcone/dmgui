#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef int dmgui_font_t;

typedef struct DmguiFontVertex
{
    dmgui_floatv2_t pos;
    dmgui_floatv2_t uv;
    uint32_t color;

    // optional: could embed in texture lookup by segIdx
    uint32_t segIdx;
    uint32_t segCount;

    // optional: could embed in texture lookup by segIdx
    int16_t bboxMinX;
    int16_t bboxMinY;
    int16_t bboxSizeX;
    int16_t bboxSizeY;
} DmguiFontVertex;

// bezier curve segment
// to fit in min spec texture size (2048x2048), need to increment row every 1024 segments
typedef struct DmguiFontSegment {
    // rgba16i
    // vertex position start[x,y] (rg), end[x,y] (ba)
    int16_t p0x; // (rgba)
    int16_t p0y;
    int16_t p1x;
    int16_t p1y;

    // rgba16i
    // control point [x,y] (rg)
    int16_t cpx; // (rg)
    int16_t cpy;

    // number of segments in the curvature - only set on first segment
    int16_t segCount; // (b)
    int16_t pad; // (a)

#ifdef __cplusplus
    DmguiFontSegment() = default;
    DmguiFontSegment(int16_t _p0x, int16_t _p0y, int16_t _p1x, int16_t _p1y, int16_t _cpx, int16_t _cpy, int16_t _segCnt, int16_t _pad)
        : p0x{_p0x}, p0y{_p0y}, p1x{_p1x}, p1y{_p1y}, cpx{_cpx}, cpy{_cpy}, segCount{_segCnt}, pad{_pad} {}
#endif
} DmguiFontSegment;

typedef struct DmguiFontBounds {
    int16_t minX;
    int16_t minY;
    int16_t maxX;
    int16_t maxY;
} DmguiFontBounds;

typedef struct DmguiFontSegments {
    const DmguiFontSegment* data;
    size_t count;
} DmguiFontSegments;

typedef struct DmguiTextVertices {
    size_t verticesCount;
    dmgui_floatv2_t bounds;
} DmguiTextVertices;

// loads and creates the font context
// return is id as index in order of creation
// note: id is recycled upon destruction
dmgui_font_t dmguiCreateFont(const char* path);
// destroy the font context and free associated memory, recycles font id
void dmguiDestroyFont(dmgui_font_t font);

////
// Below functions are used by dmgui internally 
////

dmgui_texture_t dmguiGetFontTexture(dmgui_font_t font);
// text must be null terminated utf8 compatible (ascii preferred)
size_t dmguiGetTextGlyphCount(const char* text);
// ensure your vertex buffer size can store at least this many vertices before calling dmguiGetTextVertices
// text must be null terminated utf8 compatible (ascii preferred)
// prefer dmguiGetTextMaxVerticesCount if the text length is known
size_t dmguiGetTextVerticesCount(const char* text);
// ensure your vertex buffer size can store at least this many vertices before calling dmguiGetTextVertices
inline size_t dmguiGetTextMaxVerticesCount(size_t textLen) { return textLen * 4; }
// writes vertex data to outVertices, every 4 vertices represends a glyph quad
// text must be null terminated uft8 compatible (ascii preferred)
// ensure buffer pointed to by outVertices is large enougn to store maximum of textLen*4 vertices
// returns verticesCount written, bounds of the vertices
DmguiTextVertices dmguiGetTextVertices(dmgui_font_t font, const char* text, float fontSize, uint32_t textColor, dmgui_floatv2_t offset, DmguiFontVertex* outVertices);

#ifdef __cplusplus
}
#endif