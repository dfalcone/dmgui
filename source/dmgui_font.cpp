// --
// stb_truetype dependency
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
//#define STBTT_malloc(x,u)  ((void)(u),GuiFontMalloc(x))
//#define STBTT_free(x,u)    ((void)(u),free(x))
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include "stb_truetype.h"
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
// --

#define DMGUI_IMPLEMENTATION
#include "dmgui/dmgui_config.h"
#include "dmgui/dmgui.h"
#include "dmgui/dmgui_font.h"

#include <stdio.h>

// implemented in render backend
dmgui_texture_t dmguiCreateFontTexture(uint32_t w, uint32_t h, void* data, size_t dataSize);

struct DmguiFontContext
{
    void* source;
    uint32_t sourceSize;
    stbtt_fontinfo info;
    float unitsPerEm;
    dmgui_texture_t texture;

    // cpu data cached lookups for text vertices
    // todo: investigate data structures for non-latin1 languages (flat map? trie?)
    // array lookup would be 4mb per array which is too big
    // also depending on language selected could elect to only load that glyph set
    DmguiFontBounds utf8ToBounds[256];
    uint32_t utf8ToGlyphId[256];
    uint32_t uft8ToSegmentIdx[256];
    uint32_t uft8ToSegmentCount[256];
};

static dmgui_vector_t<DmguiFontContext*> s_fontContexts;
static dmgui_vector_t<int> s_fontContextsFree;

static void GetGlyphCurves(DmguiFontContext* font, DmguiFontSegment* segments, uint32_t& segmentCount, int32_t unicodeChar);

dmgui_font_t dmguiCreateFont(const char* path)
{
    // load ttf to memory
    FILE* file = fopen(path, "rb");
    if (!file) {
        printf("finle not found [%s]", path);
        assert(0 && "file not found");
        return -1;
    }
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    DmguiFontContext* font = (DmguiFontContext*)dmguiMalloc(sizeof(DmguiFontContext) + fileSize);
    memset(font, 0, sizeof(DmguiFontContext));
    new (font) DmguiFontContext{}; // todo: get rid of segments vector to remove constructor need
    font->source = font + 1; // ttf memory immediately after DmguiFont
    font->sourceSize = fileSize;

    fread(font->source, font->sourceSize, 1, file);
    fclose(file);

    // parse ttf
    stbtt_InitFont(&font->info, (const uint8_t*)font->source,
        stbtt_GetFontOffsetForIndex((const uint8_t*)font->source, 0));

    // units per em is in the TTF, stb_truetype does not expose it so extract manually from header
    // store in a uniform so shader can get scale as fontSize / unitsPerEm
    const unsigned char* pUnitsPerEm = font->info.data + font->info.head + 18;
    const uint16_t unitsPerEm = (pUnitsPerEm[0] << 8) | pUnitsPerEm[1];
    font->unitsPerEm = (float)unitsPerEm;

    // store utf8 to glyph index map (optional)

    // temp buffer for segments texture data
    constexpr uint32_t maxDim = 2048;
    constexpr size_t maxTetureSize = maxDim*maxDim*8; // each texel is int16_t[4] (8 bytes)
    DmguiFontSegment* segments = (DmguiFontSegment*)dmguiMalloc(maxTetureSize);
    uint32_t segmentCount = 0;

    // create runtime buffers for cpu and gpu (only latin languages for now)
    // this is temporary
    for (int32_t i = 0; i != 256; ++i) {
        //int32_t i = 'a';
        assert(font->uft8ToSegmentIdx[i] == 0);
        font->uft8ToSegmentIdx[i] = segmentCount;
        GetGlyphCurves(font, segments, segmentCount, i);
        assert(font->uft8ToSegmentCount[i] == 0);
        font->uft8ToSegmentCount[i] = segmentCount - font->uft8ToSegmentIdx[i];
    }

    constexpr uint32_t maxDimHalf = maxDim / 2;
    const uint32_t maxHeight = segmentCount / maxDimHalf + 1;
    const uint32_t textureHalfDimSize = maxDimHalf * maxHeight; // each segment is 2 texels
    const uint32_t textureSize = textureHalfDimSize * sizeof(DmguiFontSegment);
    font->texture = dmguiCreateFontTexture(maxDim, maxHeight, segments, textureSize);

    dmguiFree(segments);

    dmgui_font_t fontId;
    if (s_fontContextsFree.empty()) {
        fontId = (dmgui_font_t)s_fontContexts.size();
        s_fontContexts.push_back(font);
    }
    else {
        fontId = s_fontContextsFree.back();
        s_fontContextsFree.pop_back();
        s_fontContexts[fontId] = font;
    }

    return fontId;
}

void dmguiDestroyFont(dmgui_font_t fontId) {
    s_fontContextsFree.push_back(fontId);
    dmguiFree(s_fontContexts[fontId]);
    s_fontContexts[fontId] = NULL;
}

dmgui_texture_t dmguiGetFontTexture(dmgui_font_t font) {
    return s_fontContexts[font]->texture;
}

static void GetGlyphCurves(DmguiFontContext* font, DmguiFontSegment* segments, uint32_t& segmentCount, int32_t unicodeChar) {
    // todo store these in a map
    int glyphIdx = stbtt_FindGlyphIndex(&font->info, unicodeChar);
    font->utf8ToGlyphId[unicodeChar] = (uint32_t)glyphIdx;

    // todo - own parser could store these as int16_t
    int x0, y0, x1, y1;
    stbtt_GetGlyphBox(&font->info, glyphIdx, &x0, &y0, &x1, &y1);
    font->utf8ToBounds[unicodeChar] = { (int16_t)x0, (int16_t)y0, (int16_t)x1, (int16_t)y1 };

    stbtt_vertex* vertices{};
    DmguiFontSegment* segmentsEnd = segments + segmentCount;
    int vn = stbtt_GetGlyphShape(&font->info, glyphIdx, &vertices); // calls malloc
    if (vn > 0) {
        assert(vertices);
        assert(vertices[0].type == STBTT_vmove);
        int16_t vx0 = vertices[0].x;
        int16_t vy0 = vertices[0].y;
        int16_t cpx, cpy;
        size_t ci = segmentsEnd - segments;
        uint32_t segCount = 0;
        for (int vi = 1; vi < vn; ++vi) {
            const stbtt_vertex& v = vertices[vi];
            switch (v.type)
            {
            case STBTT_vmove:
                assert(segCount > 0);
                assert(segments[ci].segCount == 0);
                segments[ci].segCount = (int16_t)segCount;
                ci += segCount;
                assert(ci > 0);
                segCount = 0;
                //ci = (int)ctx->segments.size();
                break;
            case STBTT_vline:
                cpx = (vx0 + v.x) / 2;
                cpy = (vy0 + v.y) / 2;
                *segmentsEnd = { vx0, vy0, (int16_t)v.x, (int16_t)v.y, (int16_t)cpx, (int16_t)cpy, (int16_t)0, (int16_t)0 };
                ++segmentsEnd;
                ++segCount;
                break;
            case STBTT_vcurve:
                *segmentsEnd = { vx0, vy0, v.x, v.y, v.cx, v.cy, 0, 0 };
                ++segmentsEnd;
                ++segCount;
                break;
            case STBTT_vcubic:
                cpx = (v.cx + v.cx1) / 2;
                cpy = (v.cy + v.cy1) / 2;
                *segmentsEnd = { vx0, vy0,v.x, v.y, cpx, cpy, 0, 0 };
                ++segmentsEnd;
                ++segCount;
                break;
            }

            vx0 = (float)v.x;
            vy0 = (float)v.y;
        }

        assert(segments[ci].segCount == 0);
        segments[ci].segCount = segCount;

        //{ // validation
        //    int si = 0;
        //    int segEnd = 0;
        //    while (si < font->segments.size()) {
        //        segCount = font->segments[si].segCount;
        //        segEnd = si + segCount;
        //        ++si;
        //        for (; si < segEnd; ++si)
        //            assert(font->segments[si].segCount == 0);
        //    }
        //}
    }
    stbtt_FreeShape(&font->info, vertices);
    segmentCount = segmentsEnd - segments;
}

static inline uint32_t SizeOfUtf8(const uint8_t b) {
    return (b < 0x80) ? 1 :
           (b < 0xE0) ? 2 :
           (b < 0xF0) ? 3 : 4;
}

static const uint8_t first_byte_mask[5] = {
    0x7F, // 1-byte: 0xxxxxxx
    0x1F, // 2-byte: 110xxxxx
    0x0F, // 3-byte: 1110xxxx
    0x07, // 4-byte: 11110xxx
    0x00  // fallback, should never be used
};

static inline uint32_t DecodeUtf8Codepoint(const uint8_t *s, uint32_t size) {
    // Mask first byte to remove UTF-8 prefix
    uint32_t cp = s[0] & first_byte_mask[size - 1];
    // Append continuation bytes
    if (size > 1) cp = (cp << 6) | (s[1] & 0x3F);
    if (size > 2) cp = (cp << 6) | (s[2] & 0x3F);
    if (size > 3) cp = (cp << 6) | (s[3] & 0x3F);
    return cp;
}

size_t dmguiGetTextGlyphCount(const char* text) {
    size_t len = 0;
    const uint8_t* utf8Str = (const uint8_t*)text;
    while (const uint8_t c = *utf8Str) {
        const uint32_t codepointSize = SizeOfUtf8(c);
        //const uint32_t codepoint = DecodeUtf8Codepoint(utf8Str, codepointSize);
        utf8Str += codepointSize;
        ++len;
    }
    return len;
}

size_t dmguiGetTextVerticesCount(const char* text) {
    size_t len = 0;
    const uint8_t* utf8Str = (const uint8_t*)text;
    while (const uint8_t c = *utf8Str) {
        const uint32_t codepointSize = SizeOfUtf8(c);
        //const uint32_t codepoint = DecodeUtf8Codepoint(utf8Str, codepointSize);
        utf8Str += codepointSize;
        ++len;
    }
    return len * 4;
}

// public api
// returns string len / glyph count
DmguiTextVertices dmguiGetTextVertices(dmgui_font_t fontId, const char* text, float fontSize, uint32_t textColor, dmgui_floatv2_t posOffset, DmguiFontVertex* outVertices) {
    DmguiTextVertices ret{};
    DmguiFontContext* font = s_fontContexts[fontId];
    //GuiFontBounds stringBounds = { FLT_MAX, FLT_MAX, FLT_MIN, FLT_MIN };

    float scale = stbtt_ScaleForPixelHeight(&font->info, fontSize);
    //float scale = stbtt_ScaleForMappingEmToPixels(&fontinfo, fontSize);

    int ascent;  // Y distance from text baseline to top (above line)
    int descent; // Y distance from text baseline to bottom (bellow line)
    int lineGap; // Y distance between separate lines and wrapped text
    stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &lineGap);
    const float baseline = ascent * scale;

    // only used for word wrap
    float lineHeight = (float)(ascent - descent + lineGap) * scale;

    // offset glyph bounds so that the top of bounds is at y=0
    float offsetY = -(float)ascent * scale;

    float offsetX = 0.f;
    uint32_t glyphIdLast = 0; // for kerning
    const uint8_t* utf8Str = (const uint8_t*)text;
    while (const uint8_t c = *utf8Str) {
        const uint32_t codepointSize = SizeOfUtf8(c);
        const uint32_t codepoint = DecodeUtf8Codepoint(utf8Str, codepointSize);

        const int glyphId = font->utf8ToGlyphId[codepoint];
        offsetX += glyphIdLast ? (float)stbtt_GetGlyphKernAdvance(&font->info, glyphIdLast, glyphId) : 0.f;

        const DmguiFontBounds bounds = font->utf8ToBounds[codepoint];
        const float posXMin = posOffset.x + (offsetX + (float)bounds.minX) * scale;
        const float posXMax = posOffset.x + (offsetX + (float)bounds.maxX) * scale;
        const float posYMin = posOffset.y + baseline - (offsetY + (float)bounds.minY) * scale;
        const float posYMax = posOffset.y + baseline - (offsetY + (float)bounds.maxY) * scale;
        ret.bounds.x += posXMax - posXMin;
        ret.bounds.y += posYMax - posYMin;

        DmguiFontVertex v{};
        v.color = textColor;
        v.segIdx = font->uft8ToSegmentIdx[codepoint];
        v.segCount = font->uft8ToSegmentCount[codepoint];
        v.bboxMinX = bounds.minX;
        v.bboxMinY = bounds.minY;
        v.bboxSizeX = bounds.maxX - bounds.minX;
        v.bboxSizeY = bounds.maxY - bounds.minY;

        v.pos = { posXMin, posYMin };
        v.uv = { 0.f, 0.f }; // todo: can just scale UVs to font space here on cpu or cache
        outVertices[ret.verticesCount++] = v;

        v.pos = { posXMax, posYMin };
        v.uv = { 1.f, 0.f };
        outVertices[ret.verticesCount++] = v;

        v.pos = { posXMax, posYMax };
        v.uv = { 1.f, 1.f };
        outVertices[ret.verticesCount++] = v;

        v.pos = { posXMin, posYMax };
        v.uv = { 0.f, 1.f };
        outVertices[ret.verticesCount++] = v;

        int advancedWidth, leftSideBearing;
        stbtt_GetGlyphHMetrics(&font->info, glyphId, &advancedWidth, &leftSideBearing);
        offsetX += (float)advancedWidth;

        glyphIdLast = glyphId;
        utf8Str += codepointSize;
    }

    return ret;
}
