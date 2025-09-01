#pragma once

#include <stddef.h>
#include <stdint.h>

typedef char dmgui_id_t[16];

#ifdef DMGUI_ENABLE_32BIT_INDICES
typedef uint32_t dmgui_index_t;
#else
typedef uint16_t dmgui_index_t; // max 65536
#endif

typedef struct dmgui_floatv2_t { float x, y; } dmgui_floatv2_t;
typedef struct dmgui_floatv3_t { float x, y, z; } dmgui_floatv3_t;
typedef struct dmgui_floatv4_t { float x, y, z, w; } dmgui_floatv4_t;
typedef struct dmgui_fcolor_t { float r, g, b, a; } dmgui_fcolor_t;
typedef struct dmgui_color_t { uint8_t r, g, b, a; } dmgui_color_t;

typedef void(*DMGUI_CALLBACK_HOVER)(dmgui_index_t idx);
typedef void(*DMGUI_CALLBACK_PRESS)(dmgui_index_t idx);
typedef void(*DMGUI_CALLBACK_RELEASE)(dmgui_index_t idx);
typedef void(*DMGUI_CALLBACK_DRAG)(dmgui_index_t idx, dmgui_floatv2_t posDelta);

#include DMGUI_RENDER_H
#include DMGUI_INPUT_H

// ----------------------------------------------------------------------------
// Components

typedef enum DMGUI_COMPONENT
{
    DMGUI_COMPONENT_FILL,
    DMGUI_COMPONENT_FILLV,

    DMGUI_COMPONENT_COLOR,
    DMGUI_COMPONENT_COLORV,
    DMGUI_COMPONENT_TEXCOORDV,
    DMGUI_COMPONENT_TEX,

    DMGUI_COMPONENT_RECT,
    DMGUI_COMPONENT_IMG,

    DMGUI_COMPONENT_MARGIN,
    DMGUI_COMPONENT_PADDING,
    DMGUI_COMPONENT_BORDER,
    DMGUI_COMPONENT_TITLEBAR,

    DMGUI_COMPONENT_BIND_HOVER,
    DMGUI_COMPONENT_BIND_PRESS,
    DMGUI_COMPONENT_BIND_RELEASE,
    DMGUI_COMPONENT_BIND_DRAG,

    DMGUI_COMPONENT_FLEX_CONTAINER,
    DMGUI_COMPONENT_FLEX_ITEM,

    DMGUI_COMPONENT_TEXT,
} DMGUI_COMPONENT;

typedef struct DmguiRect
{
    dmgui_floatv2_t min;
    dmgui_floatv2_t max;
    dmgui_floatv2_t uv[4];
    dmgui_color_t color[4];
    dmgui_texture_t texture;

#ifdef __cplusplus
    DmguiRect() :
        min{},
        max{},
        uv{ {0.f,0.f}, {1.f,0.f}, {1.f,1.f}, {0.f,1.f} },
        color{ {255,255,255,255}, {255,255,255,255}, {255,255,255,255}, {255,255,255,255} },
        texture{ DMGUI_TEXTURE_DEFAULT }
    {}
#endif
} DmguiRect;

typedef enum DMGUI_FLEX_JUSTIFY
{
    // css default
    DMGUI_FLEX_JUSTIFY_START,
    DMGUI_FLEX_JUSTIFY_END,
    DMGUI_FLEX_JUSTIFY_CENTER,
    DMGUI_FLEX_JUSTIFY_SPACE_BETWEEN,
    DMGUI_FLEX_JUSTIFY_SPACE_AROUND,
    DMGUI_FLEX_JUSTIFY_SPACE_EVENLY,

    // not implemented:
    // DMGUI_FLEX_JUSTIFY_FLEXSTART,
    // DMGUI_FLEX_JUSTIFY_FLEXEND,
    // DMGUI_FLEX_JUSTIFY_LEFT,
    // DMGUI_FLEX_JUSTIFY_RIGHT,
} DMGUI_FLEX_JUSTIFY;

typedef enum DMGUI_FLEX_ALIGN
{
    // css default - items stretch to fill container
    DMGUI_FLEX_ALIGN_STRETCH,
    // items placed along start of cross axis edge - top, bottom, left, right
    DMGUI_FLEX_ALIGN_START,
    // items placed along end of cross axis edge - bottom, top, righ, left
    DMGUI_FLEX_ALIGN_END,
    // items placed center of cross axis edge
    DMGUI_FLEX_ALIGN_CENTER,
    // none - wondering if this should be the default or keep css standard
    DMGUI_FLEX_ALIGN_BASELINE,

    // not implemented:
    // DMGUI_FLEX_ALIGN_FLEXSTART,
    // DMGUI_FLEX_ALIGN_FLEXEND,
    // DMGUI_FLEX_ALIGN_FIRSTBASELINE,
    // DMGUI_FLEX_ALIGN_LASTBASELINE,
    // DMGUI_FLEX_ALIGN_SELFSTART,
    // DMGUI_FLEX_ALIGN_SELFEND,
} DMGUI_FLEX_ALIGN;

typedef struct DmguiFlexContainerComponent
{
    // direction goes right to left or bottom to top with column
    int dirReverse : 1;
    // dircteon goes top to bottom or bottom to top with reverse
    int dirColumn : 1;

    // 0 none, 1 wrap, -1 wrap reverse
    int wrap : 4;

    // justify main axis
    enum DMGUI_FLEX_JUSTIFY justifyContent : 8;
    // justify cross axis
    enum DMGUI_FLEX_JUSTIFY alignContent : 8;
    // align cross axis
    enum DMGUI_FLEX_ALIGN alignItems : 8;

    // space between items, does not apply to outer space
    float gap;
} DmguiFlexContainerComponent;

typedef struct DmguiFlexItemComponent
{
    // relative size factor when growing to fill remaining space
    float grow;
    // relative size factor when shrinking to allow other items to grow
    float shrink;
    // default size before remaining space distributed, ignored if {0,0}
    dmgui_floatv2_t basis;
    // min size for item when shrinking
    dmgui_floatv2_t min;
    // max size for item when growing
    dmgui_floatv2_t max;

    // not implemented
    // int order; // rearanges the items in layout, not allowed
    // GUI_FLEX_ALIGN alginSelf; // overrides container alignItems for this item, not allowed
    // flexBasis auto: 0 (default) vs auto for whether affects item content padding
} DmguiFlexItemComponent;

typedef struct DmguiTextComponent
{
    // ascii or utf8 null terminated string
    const char* text;
    dmgui_color_t textColor;
    // pixel height of font
    float fontSize;
    // id acquired from dmguiCreateFont
    // as index in order of creation (stable values)
    // if destroyed, index is recycled
    int fontId;
} DmguiTextComponent;

typedef struct DmguiComponent
{
    enum DMGUI_COMPONENT type : 8;
    uint8_t bindButton;
    uint8_t style;
    union
    {
        dmgui_color_t color;
        const dmgui_color_t* colorv;
        dmgui_floatv2_t padding;
        dmgui_floatv2_t margin;
        dmgui_texture_t texture;
        const dmgui_floatv2_t* texcoordv;
        const DmguiFlexContainerComponent* flexContainer;
        const DmguiFlexItemComponent* flexItem;
        DmguiTextComponent* text;
        union
        {
            void* bindCallback;
            DMGUI_CALLBACK_HOVER bindCallbackHover;
            DMGUI_CALLBACK_PRESS bindCallbackPress;
            DMGUI_CALLBACK_RELEASE bindCallbackRelease;
            DMGUI_CALLBACK_DRAG bindCallbackDrag;
        };
    };
} DmguiComponent;

// ----------------------------------------------------------------------------
// Objects

typedef struct DmguiObject
{
    const dmgui_id_t* id;
    dmgui_floatv2_t pos;
    dmgui_floatv2_t size;
    bool noConsume;
    size_t componentCount;
    const DmguiComponent* components;
} DmguiObject;

typedef struct DmguiObjects
{
    const dmgui_id_t* id;
    dmgui_floatv2_t pos;
    dmgui_floatv2_t size;
    bool noConsume;
    size_t componentCount;
    const DmguiComponent* components;
    size_t childObjectCount;
    const struct DmguiObjects* childObjects;
} DmguiObjects;

// ----------------------------------------------------------------------------
// API

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dmgui_context_t dmgui_context_t;
dmgui_context_t* dmguiCreateContext(
    dmgui_floatv2_t viewportResolution,
    dmgui_floatv2_t canvasResolution,
    const DmguiParamCreateRenderContext* renderContext,
    const DmguiParamCreateInputContext* inputContext);
void dmguiDestroyContext(dmgui_context_t*);
void dmguiSetContext(dmgui_context_t*);
void dmguiSetViewportResolution(dmgui_floatv2_t v);
void dmguiSetCanvasResolution(dmgui_floatv2_t v);
void dmguiUpdateContextBegin();
void dmguiUpdateContextEnd();
void dmguiBeginChild();
void dmguiEndChild();
void dmguiDrawObject(const DmguiObject* in);
void dmguiDrawObjects(const DmguiObjects* in);

#ifdef __cplusplus
} // extern "C"
#endif
