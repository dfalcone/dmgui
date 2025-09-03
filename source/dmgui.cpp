
#define DMGUI_IMPLEMENTATION
#include "dmgui/dmgui_config.h"
#include "dmgui/dmgui.h"
#include "dmgui/dmgui_font.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define DMGUI_OBJECT_FLAG_CONSUME       0x1     // whether it should consume input
#define DMGUI_OBJECT_FLAG_MOVE          0x2     // whether it can be moved
//#define DMGUI_OBJECT_FLAG_INTERSECT   0x4     // whether the input pos intersects bounds
#define DMGUI_OBJECT_FLAG_BIND_PRESS    0x8     //
#define DMGUI_OBJECT_FLAG_BIND_RELEASE  0x10    //
#define DMGUI_OBJECT_FLAG_BIND_DRAG     0x20    //
#define DMGUI_OBJECT_FLAG_BIND_HOVER    0x40    //
#define DMGUI_OBJECT_FLAG_8             0x80    //

// ----------------------------------------------------------------------------
// Internal Components

struct Id {
    dmgui_id_t val;
    //inline Id(const dmgui_id_t& v) { memcpy(&val, &v, sizeof(dmgui_id_t)); }
};
static inline bool operator==(const Id& l, const Id& r) { return memcmp(&l, &r, sizeof(Id)) == 0; }

struct Bounds
{
    dmgui_floatv2_t min;
    dmgui_floatv2_t max;
};

struct VertexInfo
{
    uint32_t offset;
    uint32_t count : 24;
    uint32_t size : 8;
    VertexInfo* next;

    //VertexInfo(uint32_t _off, uint32_t _cnt, uint32_t _sz, VertexInfo* _nxt) :  offset{_off}, count{_cnt}, size{_sz}, next(_nxt) {}
    VertexInfo(uint32_t _off, uint32_t _cnt, uint32_t _sz, VertexInfo* _nxt) :  offset{_off}, count{_cnt}, size{_sz}, next(_nxt) {}
};

union ButtonBindMapKey
{
    uint32_t val;
    struct
    {
        uint16_t idx;
        uint8_t type;
        uint8_t button;
    };
};

// ----------------------------------------------------------------------------
// Context

struct dmgui_context_t {
    dmgui_floatv2_t viewport;
    dmgui_floatv2_t canvas;

    DmguiRenderContext render;
    DmguiInputContext input;

    dmgui_vector_t<DmguiDraw> draws;
    dmgui_vector_t<uint8_t> vertices;
    dmgui_vector_t<dmgui_index_t> indices;

    dmgui_vector_t<uint8_t> verticesText;
    dmgui_vector_t<dmgui_index_t> indicesText;

    dmgui_floatv2_t drawPos;
    dmgui_vector_t<dmgui_floatv2_t> drawPosStack; // todo: optional fixed MAX_GUI_SCOPES

    dmgui_index_t parentIdxCandidate = (dmgui_index_t)-1;
    dmgui_vector_t<dmgui_index_t> parentIdxStack; // todo: optional fixed MAX_GUI_SCOPES

    dmgui_index_t objectCount = 0;
    // the id of the object that was being pressed last frame
    Id pressIdLast{0};

    struct Components
    {
        // required components
        // always naturally sorted in draw order
        // todo: use single allocation
        dmgui_vector_t<Bounds> bounds;
        dmgui_vector_t<uint8_t> flags;
        dmgui_vector_t<Id> id;
        dmgui_vector_t<dmgui_index_t> parentIdx;
        dmgui_vector_t<VertexInfo> vertexInfo;

        // optional components - linked
        dmgui_vector_t<VertexInfo> vertexInfoNext;

        // optional components - indirect
        // each component has a corresponding lookup for obj idx to com idx
        // todo: modify handle map to allow emplace of specified id
        dmgui_vector_t<dmgui_index_t> flexIdx;
        dmgui_vector_t<DmguiFlexItemComponent> flex;

        // optional components - hash maps
        // todo: linear open address map - ~O(1)(miss) insert ~O(1)(miss) lookup
        dmgui_unordered_map_t<uint32_t /*ButtonBindMapKey*/, void* /*callback*/> bindings; // todo: create flat map
    } coms;

    // todo: perf: all scope stacks share allocation
    // todo: perf: config compile option for fixed stack sizes
    struct Layout
    {
        // optional layout components
        // next vars hold vals for the last object created
        // next vars promoted to stack in BeginGuiChild and popped back to next in EndGuiChild

        // todo: possible to just use stack and pop next on each obj if its not the parent's?
        // could keep a bool set on begin to not pop for first child
        dmgui_floatv2_t paddingCandidate;
        // todo: optional - fixed size array MAX_GUI_SCOPES
        dmgui_vector_t<dmgui_floatv2_t> paddingStack;

        // set to true when component encountered
        // push to stack in begin
        // set to stack val and pop stack in end
        bool isFlex = false;
        // todo: optional - fixed size array MAX_GUI_SCOPES
        dmgui_vector_t<bool> isFlexStack;

        // pushed when component encountered, set isFlex true
        // pop in end if isFlex true
        // todo: optional - fixed size array MAX_GUI_SCOPES
        dmgui_vector_t<DmguiFlexContainerComponent> flexContainerStack;

        // objectCount at BeginGuiChild, this is the child start idx
        dmgui_vector_t<dmgui_index_t> objectCount;
    } layout;
};

struct DmguiStyleContext
{
    dmgui_color_t color[DMGUI_STYLE_COUNT];
    int img[DMGUI_STYLE_COUNT];
    dmgui_color_t fontColor[DMGUI_STYLE_COUNT];
    float fontSize[DMGUI_STYLE_COUNT];
    int font[DMGUI_STYLE_COUNT];
    dmgui_color_t colorv[DMGUI_STYLE_COUNT][4];
};

// ----------------------------------------------------------------------------
// Data

static dmgui_context_t* s_ctx = 0;
static DmguiStyleContext* s_style = 0;
static dmgui_vector_t<dmgui_context_t*> s_contexts;

// ----------------------------------------------------------------------------
// Static Functions

static inline bool operator==(const dmgui_floatv2_t& l, const dmgui_floatv2_t& r) { return l.x == r.x && l.y == r.y; }
static inline dmgui_floatv2_t operator+(const dmgui_floatv2_t& l, const dmgui_floatv2_t& r) { return { l.x + r.x, l.y + r.y }; }
static inline dmgui_floatv2_t operator-(const dmgui_floatv2_t& l, const dmgui_floatv2_t& r) { return { l.x - r.x, l.y - r.y }; }
static inline dmgui_floatv2_t operator*(const dmgui_floatv2_t& l, const dmgui_floatv2_t& r) { return { l.x * r.x, l.y * r.y }; }
static inline dmgui_floatv2_t operator/(const dmgui_floatv2_t& l, const dmgui_floatv2_t& r) { return { l.x / r.x, l.y / r.y }; }
static inline dmgui_floatv2_t operator*(const dmgui_floatv2_t& l, const float r) { return { l.x * r, l.y * r }; }
static inline dmgui_floatv2_t operator/(const dmgui_floatv2_t& l, const float r) { return { l.x / r, l.y / r }; }

static dmgui_floatv2_t ViewportToCanvas(dmgui_floatv2_t canvasSize, dmgui_floatv2_t viewportSize, dmgui_floatv2_t viewportPos) {
    float canvasAspect   = canvasSize.x / canvasSize.y;
    float viewportAspect = viewportSize.x / viewportSize.y;

    // Letterbox if canvas is wider than viewport, else pillarbox
    float letterbox = (canvasAspect > viewportAspect); // 1.f if letterbox, else 0.f
    float pillarbox = 1.f - letterbox;

    // Base scale from canvas units to NDC [-1, 1]
    dmgui_floatv2_t baseScale = { 2.f / canvasSize.x, 2.f / canvasSize.y };

    // Uniform scale factor
    float uniformScale =
        (viewportAspect / canvasAspect) * letterbox +
        (canvasAspect / viewportAspect) * pillarbox;

    // Final scale used in projection
    dmgui_floatv2_t s = {
        baseScale.x * uniformScale * pillarbox + baseScale.x * letterbox,
        baseScale.y * uniformScale * letterbox + baseScale.y * pillarbox
    };

    // Translation offset used in projection
    dmgui_floatv2_t offset = {
        (1.f - uniformScale) * pillarbox,
        (1.f - uniformScale) * letterbox
    };

    // Convert viewport position (in pixels) to NDC
    dmgui_floatv2_t ndcPos = {
        (viewportPos.x / viewportSize.x) * 2.f - 1.f,
        (viewportPos.y / viewportSize.y) * 2.f - 1.f
    };

    // Invert the projection to get canvas position
    dmgui_floatv2_t canvasPos = {
        (ndcPos.x + 1.f - offset.x) / s.x,
        (ndcPos.y + 1.f - offset.y) / s.y
    };

    return canvasPos;
}

// this function is written in a way to make it easy for compiler SSE/Neon optimizations
static void ToProj(dmgui_floatv2_t canvasSize, dmgui_floatv2_t viewportSize, dmgui_floatv4_t out[4]) {
    const float canvasAspect   = canvasSize.x / canvasSize.y;
    const float viewportAspect = viewportSize.x / viewportSize.y;

    // letterbox if canvas wider than viewport, else pillarbox
    const float letterbox = (float)(canvasAspect > viewportAspect);
    const float pillarbox = 1.f - letterbox;

    // base scale from canvas units to NDC [-1, 1]
    const dmgui_floatv2_t baseScale = {
        2.f / canvasSize.x,
        2.f / canvasSize.y
    };

    // uniform normalized scale
    const float uniformScale =
        (viewportAspect / canvasAspect) * letterbox +
        (canvasAspect / viewportAspect) * pillarbox;

    // apply uniform scale to correct axis
    const dmgui_floatv2_t s = {
        baseScale.x * uniformScale * pillarbox + baseScale.x * letterbox,
        baseScale.y * uniformScale * letterbox + baseScale.y * pillarbox
    };

    // calculate translation offsets to center canvas in NDC [-1, 1]
    const dmgui_floatv2_t offset = {
        (1.f - uniformScale) * pillarbox,
        (1.f - uniformScale) * letterbox
    };

    // translation
    const dmgui_floatv2_t t = {
        -1.f + offset.x,
        -1.f + offset.y
    };

    // 3x3 2d ortho projection as 4x4
    out[0] = { s.x, 0.f, 0.f, 0.f };
    out[1] = { 0.f, s.y, 0.f, 0.f };
    out[2] = { t.x, t.y, 1.f, 0.f };
    out[3] = { 0.f, 0.f, 0.f, 1.f };
}

static inline bool IsPointInBounds(dmgui_floatv2_t p, dmgui_floatv2_t min, dmgui_floatv2_t max) {
    return min.x <= p.x && min.y <= p.y && max.x >= p.x && max.y >= p.y;
}

// HOT
static void UpdateInteractables() {
    size_t idx;
    const size_t count = s_ctx->objectCount;

    if (count == 0)
    {
        s_ctx->pressIdLast = {0};
        return;
    }

    const dmgui_floatv2_t inputPosViewport = {
        s_ctx->input.pos.x > s_ctx->viewport.x ? s_ctx->viewport.x : s_ctx->input.pos.x < 0.f ? 0.f : s_ctx->input.pos.x,
        s_ctx->input.pos.y > s_ctx->viewport.y ? s_ctx->viewport.y : s_ctx->input.pos.y < 0.f ? 0.f : s_ctx->input.pos.y };
    const dmgui_floatv2_t inputPosLastViewport = {
        s_ctx->input.posLast.x > s_ctx->viewport.x ? s_ctx->viewport.x : s_ctx->input.posLast.x < 0.f ? 0.f : s_ctx->input.posLast.x,
        s_ctx->input.posLast.y > s_ctx->viewport.y ? s_ctx->viewport.y : s_ctx->input.posLast.y < 0.f ? 0.f : s_ctx->input.posLast.y };
    const dmgui_floatv2_t inputPos = ViewportToCanvas(s_ctx->canvas, s_ctx->viewport, inputPosViewport);
    const dmgui_floatv2_t inputPosLast = ViewportToCanvas(s_ctx->canvas, s_ctx->viewport, inputPosLastViewport);
    const dmgui_floatv2_t inputMove = { inputPos.x - inputPosLast.x, inputPos.y - inputPosLast.y };

    const int holdMask = s_ctx->input.buttonHoldMask;
    const int pressMask = s_ctx->input.buttonPressMask;
    const int releaseMask = s_ctx->input.buttonReleaseMask;

    const Id pressIdLast = s_ctx->pressIdLast;

    // occlusion and action pass O(n)
    // iterate from last drawn to first drawn
    // handle any input actions for intersecting objects until one consumes input
    int consume = false;
    for (idx = count - 1; idx != -1 && consume == false; --idx)
    {
        const int flags = s_ctx->coms.flags[idx];
        if (holdMask && pressIdLast==s_ctx->coms.id[idx]) {
            const int hasOnPress = (flags & DMGUI_OBJECT_FLAG_BIND_PRESS) != 0;
            const int hasOnRelease = (flags & DMGUI_OBJECT_FLAG_BIND_RELEASE) != 0;
            const int hasOnMove = (flags & DMGUI_OBJECT_FLAG_BIND_DRAG) != 0;
            const int press = hasOnPress & (pressMask!=0);
            const int release =  hasOnRelease & (releaseMask!=0);
            const int hold = hasOnMove & ~press & ~release;
            if (hold)
            {
                consume = (flags & DMGUI_OBJECT_FLAG_CONSUME) != 0;
                ButtonBindMapKey key{};
                key.idx = (dmgui_index_t)(idx);
                key.type = DMGUI_OBJECT_FLAG_BIND_DRAG;
                key.button = (uint8_t)holdMask;
                auto found = s_ctx->coms.bindings.find(key.val);
                if (found != s_ctx->coms.bindings.end()) {
                    s_ctx->coms.flags[idx] |= DMGUI_OBJECT_FLAG_MOVE;
                    ((DMGUI_CALLBACK_DRAG)(found->second))(idx, inputMove);
                }
            }
        }
        // (unlikely)
        else if ((flags & (DMGUI_OBJECT_FLAG_BIND_PRESS|DMGUI_OBJECT_FLAG_BIND_RELEASE|DMGUI_OBJECT_FLAG_BIND_HOVER|DMGUI_OBJECT_FLAG_BIND_DRAG))
            && IsPointInBounds(inputPos, s_ctx->coms.bounds[idx].min, s_ctx->coms.bounds[idx].max)) {
            consume = (flags & DMGUI_OBJECT_FLAG_CONSUME) != 0;
            const int hasOnPress = (flags & DMGUI_OBJECT_FLAG_BIND_PRESS) != 0;
            const int hasOnRelease = (flags & DMGUI_OBJECT_FLAG_BIND_RELEASE) != 0;
            const int hasOnHover = (flags & DMGUI_OBJECT_FLAG_BIND_HOVER) != 0;
            const int hasOnMove = (flags & DMGUI_OBJECT_FLAG_BIND_DRAG) != 0;
            const int press = hasOnPress & (pressMask!=0);
            const int release =  hasOnRelease & (releaseMask!=0);
            const int hold = hasOnMove & ~press & ~release & (holdMask!=0);
            const int hover = hasOnHover & ~press & ~release & ~hold;

            // bindings (unlikely)
            ButtonBindMapKey key{};
            key.idx = (dmgui_index_t)(idx);
            if (hover) {
                key.type = DMGUI_OBJECT_FLAG_BIND_HOVER;
                auto found = s_ctx->coms.bindings.find(key.val);
                if (found != s_ctx->coms.bindings.end()) {
                    ((DMGUI_CALLBACK_HOVER)(found->second))(idx);
                }
            }
            else if (press) {
                key.type = DMGUI_OBJECT_FLAG_BIND_PRESS;
                key.button = (uint8_t)pressMask;
                auto found = s_ctx->coms.bindings.find(key.val);
                if (found != s_ctx->coms.bindings.end()) {
                    ((DMGUI_CALLBACK_PRESS)(found->second))(idx);
                }
            }
            else if ((release & (int)(pressIdLast == s_ctx->coms.id[idx])) != 0) {
                key.type = DMGUI_OBJECT_FLAG_BIND_RELEASE;
                key.button = (uint8_t)releaseMask;
                auto found = s_ctx->coms.bindings.find(key.val);
                if (found != s_ctx->coms.bindings.end()) {
                    ((DMGUI_CALLBACK_RELEASE)(found->second))(idx);
                }
            }

            if (pressMask != 0) {
                s_ctx->pressIdLast = s_ctx->coms.id[idx];
            }
        }

    }

    if (releaseMask) {
        s_ctx->pressIdLast = {0};
    }

    // if no input consumer, then correct obj
    idx = idx == -1 ? 0 : idx;

    // transformation pass - O(n)
    // in draw order starting from input consumer or begin
    // parents are always drawn before children
    // objects are transformed if their self or parent has move flag
    size_t vi, vn, objParentIdx, moveParentIdx = (dmgui_index_t)-2;
    for (; idx != count; ++idx) {
        objParentIdx = s_ctx->coms.parentIdx[idx];
        if (moveParentIdx != objParentIdx) {
            int parentMoved = (dmgui_index_t)objParentIdx < (dmgui_index_t)-2 ?
                s_ctx->coms.flags[objParentIdx] & DMGUI_OBJECT_FLAG_MOVE :
                0;
            int selfMoved = s_ctx->coms.flags[idx] & DMGUI_OBJECT_FLAG_MOVE;
            if ((parentMoved | selfMoved) == 0) {
                moveParentIdx = (dmgui_index_t)-2;
                continue;
            }
            if (parentMoved)
                moveParentIdx = objParentIdx;
            if (selfMoved)
                moveParentIdx = objParentIdx;
        }

        // translate the vertices
        const VertexInfo* vertInfo = &s_ctx->coms.vertexInfo[idx];
        do {
            vi = vertInfo->offset;
            vn = vi + vertInfo->count * vertInfo->size;
            for (; vi < vn; vi += vertInfo->size) {
                // rely on position x,y float always being at the start of all vertex layouts
                dmgui_floatv2_t* pos = (dmgui_floatv2_t*)(&s_ctx->vertices[vi]);
                *pos = { pos->x + inputMove.x, pos->y + inputMove.y };
            }
        } while ((vertInfo = vertInfo->next));
    }
}

static DmguiTextVertices DrawText(Bounds bounds, const char* text, float fontSize, dmgui_font_t font, dmgui_color_t textColor) {
    // draw
    const dmgui_material_t material = s_ctx->render.materialText;
    const dmgui_texture_t texture = dmguiGetFontTexture(font);

    const size_t vOffset = s_ctx->vertices.size();
    const size_t iOffset = s_ctx->indices.size();

    DmguiDraw* draw = &s_ctx->draws.back();
    if (draw->material != material || draw->texture != texture)
    {
        draw = draw->indicesCount ? &s_ctx->draws.emplace_back() : draw;
        assert(draw->verticesCount == 0);
        draw->verticesOffset = vOffset;
        draw->vertexSize = sizeof(DmguiFontVertex);
        draw->indicesOffset = iOffset;

        draw->material = material;
        draw->texture = texture;
        draw->mesh = s_ctx->render.meshText;
    }

    // vertices
    DmguiTextVertices vd;
    vd.verticesCount = dmguiGetTextVerticesCount(text);
    s_ctx->vertices.resize(vOffset + vd.verticesCount * sizeof(DmguiFontVertex));
    DmguiFontVertex* v = (DmguiFontVertex*)(s_ctx->vertices.data() + vOffset);
    vd = dmguiGetTextVertices(font, text, fontSize, *(uint32_t*)&textColor, bounds.min, v);
    s_ctx->vertices.resize(vOffset + vd.verticesCount * sizeof(DmguiFontVertex));

    // indices
    const size_t triCount = vd.verticesCount / 2;
    const size_t iCount = triCount * 3;
    s_ctx->indices.resize(iOffset + iCount);
    dmgui_index_t* i = s_ctx->indices.data() + iOffset;
    dmgui_index_t vOff = (dmgui_index_t)(draw->verticesCount);

    // the renderer context automatically splits up mesh per draw call
    for (size_t idx = 0; idx < iCount; idx += 6, vOff += 4) {
        i[idx + 0] = vOff + 0;
        i[idx + 1] = vOff + 1;
        i[idx + 2] = vOff + 2;

        i[idx + 3] = vOff + 0;
        i[idx + 4] = vOff + 2;
        i[idx + 5] = vOff + 3;
    }

    draw->verticesCount += vd.verticesCount;
    draw->indicesCount += iCount;

    return vd;
}

static DmguiVertex* EmplaceGuiRect(dmgui_texture_t texId) {
    const dmgui_material_t material = s_ctx->render.materialRect;
    dmgui_texture_t tex[] = { texId, s_ctx->render.textureDefault };
    texId = tex[texId == DMGUI_TEXTURE_DEFAULT];
    DmguiDraw* draw = &s_ctx->draws.back();
    if (draw->material != material || draw->texture != texId) {
        draw = draw->indicesCount ? &s_ctx->draws.emplace_back() : draw;
        assert(draw->verticesCount == 0);
        draw->verticesOffset = s_ctx->vertices.size();
        draw->vertexSize = sizeof(DmguiVertex);
        draw->indicesOffset = s_ctx->indices.size();
        draw->material = material;
        draw->mesh = s_ctx->render.meshRect;
        draw->texture = texId;
    }

    size_t vOffset = s_ctx->vertices.size();
    s_ctx->vertices.resize(vOffset + draw->vertexSize * 4);
    void* v = &s_ctx->vertices.data()[vOffset];

    size_t iCount = s_ctx->indices.size();
    s_ctx->indices.resize(iCount + 6);
    dmgui_index_t* i = &s_ctx->indices.data()[iCount];
    dmgui_index_t vOff = (dmgui_index_t)(draw->verticesCount);

    i[0] = vOff + 0;
    i[1] = vOff + 1;
    i[2] = vOff + 2;

    i[3] = vOff + 0;
    i[4] = vOff + 2;
    i[5] = vOff + 3;

    draw->verticesCount += 4;
    draw->indicesCount += 6;

    return (DmguiVertex*)v;
}

static inline void RectToVertexPos(DmguiVertex* v, dmgui_floatv4_t vMinMax) {
    dmgui_floatv2_t min = { vMinMax.x, vMinMax.y };
    dmgui_floatv2_t max = { vMinMax.z, vMinMax.w };
    v[0].pos = { min.x, min.y };
    v[1].pos = { max.x, min.y };
    v[2].pos = { max.x, max.y };
    v[3].pos = { min.x, max.y };
}

static inline void RectToVertexUV(DmguiVertex* v) {
    v[0].uv = { 0.f, 0.f };
    v[1].uv = { 1.f, 0.f };
    v[2].uv = { 1.f, 1.f };
    v[3].uv = { 0.f, 1.f };
}

static inline void RectToVertexUV(DmguiVertex* v, const dmgui_floatv2_t uvs[4]) {
    v[0].uv = uvs[0];
    v[1].uv = uvs[1];
    v[2].uv = uvs[2];
    v[3].uv = uvs[3];
}

static inline void RectToVertexColor(DmguiVertex* v, dmgui_color_t c) {
    v[0].color = c;
    v[1].color = c;
    v[2].color = c;
    v[3].color = c;
}

static inline void RectToVertexColor(DmguiVertex* v, const dmgui_color_t c[4]) {
    v[0].color = c[0];
    v[1].color = c[1];
    v[2].color = c[2];
    v[3].color = c[3];
}

void DrawGuiRectAbs(DmguiRect* info) {
    DmguiVertex* v = EmplaceGuiRect(info->texture);
    RectToVertexPos(v, {info->min.x, info->min.y, info->max.x, info->max.y});
    RectToVertexUV(v, info->uv);
    RectToVertexColor(v, info->color);
}

void DrawGuiRectAbsCol(dmgui_floatv2_t min, dmgui_floatv2_t max, dmgui_color_t color) {
    DmguiVertex* v = EmplaceGuiRect(DMGUI_TEXTURE_DEFAULT);
    RectToVertexPos(v, {min.x, min.y, max.x, max.y});
    RectToVertexUV(v);
    RectToVertexColor(v, color);
}

void DrawGuiRectAbsColV(dmgui_floatv2_t min, dmgui_floatv2_t max, const dmgui_color_t color[4]) {
    DmguiVertex* v = EmplaceGuiRect(DMGUI_TEXTURE_DEFAULT);
    RectToVertexPos(v, {min.x, min.y, max.x, max.y});
    RectToVertexUV(v);
    RectToVertexColor(v, color);
}

void DrawGuiRectAbsColTex(dmgui_floatv2_t min, dmgui_floatv2_t max, dmgui_color_t color, dmgui_texture_t gpuTextureId) {
    DmguiVertex* v = EmplaceGuiRect(gpuTextureId);
    RectToVertexPos(v, {min.x, min.y, max.x, max.y});
    RectToVertexUV(v);
    RectToVertexColor(v, color);
}

void DrawGuiRectAbsColVTex(dmgui_floatv2_t min, dmgui_floatv2_t max, const dmgui_color_t(&color)[4], dmgui_texture_t gpuTextureId) {
    DmguiVertex* v = EmplaceGuiRect(gpuTextureId);
    RectToVertexPos(v, {min.x, min.y, max.x, max.y});
    RectToVertexUV(v);
    RectToVertexColor(v, color);
}

void DrawGuiRectAbsColTexV(dmgui_floatv2_t min, dmgui_floatv2_t max, dmgui_color_t color, dmgui_texture_t gpuTextureId, const dmgui_floatv2_t uvs[4]) {
    DmguiVertex* v = EmplaceGuiRect(gpuTextureId);
    RectToVertexPos(v, {min.x, min.y, max.x, max.y});
    RectToVertexUV(v, uvs);
    RectToVertexColor(v, color);
}

void DrawGuiRectAbsColVTexV(dmgui_floatv2_t min, dmgui_floatv2_t max, const dmgui_color_t(&color)[4], dmgui_texture_t gpuTextureId, const dmgui_floatv2_t uvs[4]) {
    DmguiVertex* v = EmplaceGuiRect(gpuTextureId);
    RectToVertexPos(v, {min.x, min.y, max.x, max.y});
    RectToVertexUV(v, uvs);
    RectToVertexColor(v, color);
}

void DrawGuiRectAbsTex(dmgui_floatv2_t min, dmgui_floatv2_t max, dmgui_texture_t gpuTextureId) {
    DmguiVertex* v = EmplaceGuiRect(gpuTextureId);
    RectToVertexPos(v, {min.x, min.y, max.x, max.y});
    RectToVertexUV(v);
    RectToVertexColor(v, { 255, 255, 255, 255 });
}

void DrawGuiRectAbsTexV(dmgui_floatv2_t min, dmgui_floatv2_t max, dmgui_texture_t gpuTextureId, const dmgui_floatv2_t uvs[4]) {
    DmguiVertex* v = EmplaceGuiRect(gpuTextureId);
    RectToVertexPos(v, {min.x, min.y, max.x, max.y});
    RectToVertexUV(v, uvs);
    RectToVertexColor(v, { 255, 255, 255, 255 });
}

// ----------------------------------------------------------------------------
// API

dmgui_context_t* dmguiCreateContext(
    dmgui_floatv2_t viewportResolution,
    dmgui_floatv2_t canvasResolution,
    const DmguiParamCreateRenderContext* renderContext,
    const DmguiParamCreateInputContext* inputContext
) {
    dmgui_context_t* ctx = (dmgui_context_t*)dmguiMalloc(sizeof(dmgui_context_t));
    new (ctx) dmgui_context_t{};
    s_ctx = ctx;
    s_contexts.push_back(ctx);
    size_t count = s_contexts.size();
    if (count == 1) {
        s_style = (DmguiStyleContext*)dmguiMalloc(sizeof(DmguiStyleContext));
        memset(s_style, 0, sizeof(DmguiStyleContext));
    }
    ctx->viewport = viewportResolution;
    ctx->canvas = canvasResolution;
    dmguiCreateRenderContext(renderContext, count, &ctx->render);
    dmguiCreateInputContext(inputContext, &ctx->input);
    return ctx;
}

void dmguiDestroyContext(dmgui_context_t* ctx) {
    { // remove from end
        auto itr = s_contexts.end();
        auto begin = s_contexts.begin();
        while (itr != begin) {
            if (*(--itr) == ctx) {
                s_contexts.erase(itr);
                break;
            }
        }
    }
    size_t count = s_contexts.size();
    dmguiDestroyRenderContext(&ctx->render, count);
    if (s_ctx == ctx && count > 0) {
        s_ctx = s_contexts.front();
    }
    ctx->~dmgui_context_t();
    dmguiFree(ctx);
    if (count == 0) {
        dmguiFree(s_style);
        s_style = 0;
    }
}

void dmguiSetContext(dmgui_context_t* ctx) {
    s_ctx = ctx;
}

void dmguiSetViewportResolution(dmgui_floatv2_t v) {
    s_ctx->viewport = v;
}

void dmguiSetCanvasResolution(dmgui_floatv2_t v) {
    s_ctx->canvas = v;
}

void dmguiUpdateContextBegin() {
    dmguiUpdateInputContextBegin(&s_ctx->input);
    dmguiUpdateRenderContextBegin(&s_ctx->render, s_ctx->viewport, s_ctx->canvas,
        s_ctx->draws.data(), s_ctx->indices.empty() ? 0 : s_ctx->draws.size());

    // reset stacks and defaults for frame
    s_ctx->draws.clear();
    s_ctx->vertices.clear();
    s_ctx->indices.clear();

    s_ctx->drawPos = { 0.f, 0.f };

    s_ctx->parentIdxCandidate = -1;
    s_ctx->parentIdxStack.clear();
    s_ctx->parentIdxStack.emplace_back(-1);

    s_ctx->objectCount = 0;
    s_ctx->coms.bounds.clear();
    s_ctx->coms.flags.clear();
    s_ctx->coms.id.clear();
    s_ctx->coms.parentIdx.clear();
    s_ctx->coms.vertexInfo.clear();
    s_ctx->coms.vertexInfoNext.clear();
    s_ctx->coms.bindings.clear();

    s_ctx->layout.isFlex = false;
    s_ctx->layout.isFlexStack.clear();
    s_ctx->layout.flexContainerStack.clear();
    s_ctx->layout.paddingCandidate = {0.f,0.f};
    s_ctx->layout.paddingStack.clear();

    // setup default draw
    DmguiDraw draw{};
    draw.texture = s_ctx->render.textureDefault;
    draw.vertexSize = sizeof(DmguiVertex);
    s_ctx->draws.push_back(draw);

}

void dmguiUpdateContextEnd() {
    UpdateInteractables();
    dmguiUpdateInputContextEnd(&s_ctx->input);
    dmgui_floatv4_t projMatrix[4];
    ToProj(s_ctx->canvas, s_ctx->viewport, projMatrix);
    dmguiUpdateRenderContextEnd(&s_ctx->render, s_ctx->viewport, s_ctx->canvas, projMatrix,
        s_ctx->draws.data(), s_ctx->indices.empty() ? 0 : s_ctx->draws.size(),
        s_ctx->vertices.data(), s_ctx->vertices.size(),
        s_ctx->indices.data(), s_ctx->indices.size());
}

void dmguiBeginChild() {
    // if there are 2 child groups this breaks
    const size_t parentIdx = s_ctx->parentIdxCandidate;

    // todo: error checking compile option
    if (parentIdx == (dmgui_index_t)-1) {
        printf("Error: BeginGuiChild called when there is no parent.");
        return;
    }

    s_ctx->parentIdxStack.push_back(parentIdx);

    s_ctx->layout.objectCount.push_back(s_ctx->objectCount);

    s_ctx->drawPosStack.push_back(s_ctx->drawPos);
    // default - parent upper left bounds
    const dmgui_floatv2_t drawPos = s_ctx->coms.bounds[parentIdx].min;

    // if there is padding on the parent, it will be in next slot
    const dmgui_floatv2_t padding = s_ctx->layout.paddingCandidate;
    // touching top of stack memory, now in cpu cache
    s_ctx->layout.paddingStack.push_back(padding);

    // apply
    s_ctx->drawPos = { drawPos.x + padding.x, drawPos.y + padding.y };

    // using anchor
    // drawPoint = bounds.min + anchor * bounds.size

    // using anchor pos for padding dir
    // xDir = anchor.x < 0.5f ? 1.f : anchor.x > 0.5f ? -1.f : 0.f;
    // yDir = anchor.y < 0.5f ? 1.f : anchor.y > 0.5f ? -1.f : 0.f;

    // padding direction depending on draw direction
    // dir vals Can be -1, 0, 1
    // dir [1,0] pad [+,+]
    // dir [-1,0] pad [-,-]
    // dir [1,0] pad [+,+]
    // dir [1,0] pad [+,+]

    // padding direction depending on anchor
    // anchor vals range -1, 0, 1
    // anchor [-1,-1] pad [+,+]
    // anchor [0,-1] pad [0,+]
    // anchor [1,-1] pad [-,+]
    // anchor [-1,0] pad [+,0]
    // anchor [-1,1] pad [+,-]

    s_ctx->layout.isFlexStack.push_back(s_ctx->layout.isFlex);
}

void dmguiEndChild() {
    // todo: error checking compile option
    if (s_ctx->drawPosStack.empty()) {
        printf("Error: EndGuiChild called without a matching BeginGuiChild\n");
        return;
    }

    // apply flex layout modifications
    const bool isFlex = s_ctx->layout.isFlexStack.back();
    s_ctx->layout.isFlex = isFlex;
    s_ctx->layout.isFlexStack.pop_back();
    if (isFlex) {
        Bounds containerBounds = s_ctx->coms.bounds[s_ctx->parentIdxStack.back()];
        dmgui_floatv2_t containerSize = { containerBounds.max.x - containerBounds.min.x, containerBounds.max.y - containerBounds.min.y };

        // need the child count between begin/end
        size_t childBeginIdx = s_ctx->layout.objectCount.back();
        size_t childEndxIdx = s_ctx->objectCount;
        size_t childCount = childEndxIdx - childBeginIdx;

        // todo: could keep all flex vals needed cached on flex container com

        // need the sum of size of all the objects // todo: calc in draw, cache on stack
        dmgui_floatv2_t childrenSize{};
        for (size_t childIdx = childBeginIdx; childIdx != childEndxIdx; ++childIdx) {
            Bounds bounds = s_ctx->coms.bounds[childIdx];
            dmgui_floatv2_t size = { bounds.max.x - bounds.min.x, bounds.max.y - bounds.min.y };
            childrenSize = { childrenSize.x += size.x, childrenSize.y + size.y };
        }

        dmgui_floatv2_t availSize = { containerSize.x - childrenSize.x, containerSize.y - childrenSize.y };

        // set defualt size as the average of sum of sizes
        // // flex grow - the item should grow to fit remaining space relative to other items
        // // - cache highest priority (highP) from each object priority (objP)
        // // - for each item, offset is (baseSize / highP) * objP
        // // flex shrink - item shrinks relative to other items

        // apply flex to bounds and vertices:
        float growFactor = 1.f;
        dmgui_floatv2_t drawPos = containerBounds.min; // todo: direction
        for (size_t childIdx = childBeginIdx; childIdx != childEndxIdx; ++childIdx) {
            const Bounds bounds = s_ctx->coms.bounds[childIdx];
            const dmgui_floatv2_t baseSize = { bounds.max.x - bounds.min.x, bounds.max.y - bounds.min.y };
            const dmgui_floatv2_t pivot = bounds.min + (baseSize * 0.5f);

            const dmgui_floatv2_t availSizeMulGrowFactor = { availSize.x * growFactor, availSize.y * growFactor };
            dmgui_floatv2_t flexSize = { availSizeMulGrowFactor.x / (float)childCount, availSizeMulGrowFactor.y / (float)childCount };
            flexSize.x += flexSize.x < 0.f ? baseSize.x : 0.f;
            flexSize.y += flexSize.y < 0.f ? baseSize.y : 0.f;
            const dmgui_floatv2_t flexScale = flexSize / baseSize;

            // transform bounds
            const Bounds flexBounds = { drawPos, drawPos + flexSize }; // todo direction
            const dmgui_floatv2_t flexPivot = flexBounds.min + (flexSize * 0.5f);
            s_ctx->coms.bounds[childIdx] = flexBounds;

            // transform vertex positions
            const VertexInfo* vertexInfo = &s_ctx->coms.vertexInfo[childIdx];
            do {
                size_t vi = vertexInfo->offset;
                size_t vn = vi + vertexInfo->count * vertexInfo->size;
                for (; vi < vn; vi += vertexInfo->size) {
                    dmgui_floatv2_t* vPos = (dmgui_floatv2_t*)(&s_ctx->vertices[vi]);
                    const dmgui_floatv2_t pivotDelta = *vPos - pivot;
                    const dmgui_floatv2_t flexPos = flexPivot + (pivotDelta * flexScale);
                    *vPos = flexPos;
                }
            } while ((vertexInfo = vertexInfo->next));

            // todo: flex direction
            drawPos.x += flexSize.x;
        }

        s_ctx->layout.flexContainerStack.pop_back();
    }

    s_ctx->drawPos = s_ctx->drawPosStack.back();
    s_ctx->drawPosStack.pop_back();

    s_ctx->layout.paddingCandidate = s_ctx->layout.paddingStack.back();
    s_ctx->layout.paddingStack.pop_back();

    s_ctx->layout.objectCount.pop_back();

    s_ctx->parentIdxCandidate = s_ctx->parentIdxStack.back();
    s_ctx->parentIdxStack.pop_back();
}

static inline VertexInfo* GetVertexInfo(VertexInfo* p, uint32_t size) {
    if (p->size != size) {
        p->next = &s_ctx->coms.vertexInfoNext.emplace_back(
            (uint32_t)s_ctx->vertices.size(), 0, size, nullptr);
        p = p->next;
    }
    return p;
}

void dmguiDrawObject(const DmguiObject* in) {
    dmgui_floatv2_t drawPos = s_ctx->drawPos;

    uint8_t flags = in->noConsume ? 0 : DMGUI_OBJECT_FLAG_CONSUME;

    Bounds bounds = { drawPos + in->pos, drawPos + in->pos + in->size };
    s_ctx->coms.bounds.emplace_back(bounds);

    Id id = {0};
    memcpy(id.val, in->id, in->id ? strlen((const char*)in->id) : 0);
    s_ctx->coms.id.emplace_back(id);

    s_ctx->parentIdxCandidate = s_ctx->objectCount;
    s_ctx->coms.parentIdx.emplace_back(s_ctx->parentIdxStack.back());

    // next layout reset - if this will be a parent, coms will fill in layout vals
    s_ctx->layout.paddingCandidate = { 0.f, 0.f };

    VertexInfo* pVertInfo = &s_ctx->coms.vertexInfo.emplace_back(
        (uint32_t)s_ctx->vertices.size(), 0, (uint32_t)sizeof(DmguiVertex), nullptr);

    // default GuiObject is a white filled rect
    DmguiRect rect{};
    ButtonBindMapKey buttonKey{};

    for (const DmguiComponent* com = in->components, *comEnd = com + in->componentCount; com != comEnd; ++com) {
        DMGUI_COMPONENT comType = (DMGUI_COMPONENT)com->type;
        switch (comType) {
        case DMGUI_COMPONENT_FILL:
            pVertInfo = GetVertexInfo(pVertInfo, sizeof(DmguiVertex));
            pVertInfo->count += 4;
            DrawGuiRectAbsCol(bounds.min, bounds.max, com->style ? s_style->color[com->style] : com->color);
            break;
        case DMGUI_COMPONENT_FILLV:
            pVertInfo = GetVertexInfo(pVertInfo, sizeof(DmguiVertex));
            pVertInfo->count += 4;
            DrawGuiRectAbsColV(bounds.min, bounds.max, com->style ? s_style->colorv[com->style] : com->colorv);
            break;
        case DMGUI_COMPONENT_COLOR:
            rect.color[0] = com->color;
            rect.color[1] = com->color;
            rect.color[2] = com->color;
            rect.color[3] = com->color;
            break;
        case DMGUI_COMPONENT_COLORV:
            rect.color[0] = com->colorv[0];
            rect.color[1] = com->colorv[1];
            rect.color[2] = com->colorv[2];
            rect.color[3] = com->colorv[3];
            break;
        case DMGUI_COMPONENT_TEXCOORDV:
            rect.uv[0] = com->texcoordv[0];
            rect.uv[1] = com->texcoordv[1];
            rect.uv[2] = com->texcoordv[2];
            rect.uv[3] = com->texcoordv[3];
            break;
        case DMGUI_COMPONENT_TEX:
            rect.texture = com->texture;
            break;
        case DMGUI_COMPONENT_RECT:
            pVertInfo = GetVertexInfo(pVertInfo, sizeof(DmguiVertex));
            pVertInfo->count += 4;
            DrawGuiRectAbs(&rect);
            break;
        case DMGUI_COMPONENT_IMG:
            pVertInfo = GetVertexInfo(pVertInfo, sizeof(DmguiVertex));
            pVertInfo->count += 4;
            DrawGuiRectAbsTex(bounds.min, bounds.max, com->style ? s_style->img[com->style] : com->texture);
            break;
        case DMGUI_COMPONENT_MARGIN:
            // margin is gap to outer content (self)
            // this alters bounds for additional proceding coms
            bounds.min = bounds.min + com->margin;
            bounds.max = bounds.max - com->margin;
            break;
        case DMGUI_COMPONENT_PADDING:
            // padding is gap to inner content (children)
            // this moves the draw point for all child objects
            s_ctx->layout.paddingCandidate = com->padding;
            break;
        //case DMGUI_COMPONENT_BORDER:
        //    // option disable error checks
        //    if (!com->border) {
        //        printf("gui error: no border component for %s\n", in->id);
        //        continue;
        //    }
        //    bounds = DrawBorder(bounds, *(com->border));
        //    vertInfo.count += 16;
        //    break;
        //case DMGUI_COMPONENT_TITLEBAR:
        //    bounds = DrawTitleBar(bounds, *(com->border));
        //    vertInfo.count += 4;
        //    break;
        case DMGUI_COMPONENT_BIND_HOVER:
            buttonKey.type = DMGUI_OBJECT_FLAG_BIND_HOVER;
            goto BUTTON_BIND;
        case DMGUI_COMPONENT_BIND_PRESS:
            buttonKey.type = DMGUI_OBJECT_FLAG_BIND_PRESS;
            goto BUTTON_BIND;
        case DMGUI_COMPONENT_BIND_RELEASE:
            buttonKey.type= DMGUI_OBJECT_FLAG_BIND_RELEASE;
            goto BUTTON_BIND;
        case DMGUI_COMPONENT_BIND_DRAG:
            buttonKey.type= DMGUI_OBJECT_FLAG_BIND_DRAG;
            BUTTON_BIND:
            buttonKey.idx = (dmgui_index_t)s_ctx->objectCount;
            buttonKey.button = com->bindButton;
            flags |= buttonKey.type;
            s_ctx->coms.bindings.emplace(buttonKey.val, com->bindCallback);
            break;
        case DMGUI_COMPONENT_FLEX_CONTAINER:
            s_ctx->layout.isFlex = true;
            s_ctx->layout.flexContainerStack.emplace_back(*com->flexContainer);
            break;
        case DMGUI_COMPONENT_FLEX_ITEM:
            // todo
            break;
        case DMGUI_COMPONENT_TEXT: {
            // todo:
            // option to clip out of bounds
            // option to grow bounds
            // option to justify text (move vertex positions based on text bounds vs obj bounds)
            pVertInfo = GetVertexInfo(pVertInfo, sizeof(DmguiFontVertex));
            DmguiTextVertices vd = DrawText(
                bounds,
                com->text->text,
                com->style ? s_style->fontSize[com->style] : com->text->fontSize,
                com->style ? s_style->font[com->style] :com->text->fontId,
                com->style ? s_style->fontColor[com->style] :com->text->textColor);
            pVertInfo->count += vd.verticesCount;
            break; }
        default:
            assert(0 && "dmgui component type not implemented");
#if defined(__clang__) || defined(__GNUC__)
            __builtin_unreachable();
#elif defined(_MSC_VER)
            __assume(0);
#elif __cpp_lib_unreachable
            std::unreachable();
#endif
        }
    }

    s_ctx->coms.flags.push_back(flags);
    ++s_ctx->objectCount;
}

void dmguiDrawObjects(const DmguiObjects* in) {
    dmguiDrawObject((const DmguiObject*)in);
    for (size_t i = 0, n = in->childObjectCount; i < n; ++i) {
        dmguiBeginChild();
        dmguiDrawObjects(&in->childObjects[i]);
        dmguiEndChild();
    }
}
