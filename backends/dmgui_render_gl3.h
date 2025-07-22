#pragma once

typedef uint32_t dmgui_mesh_t; // GLuint vao[0], vertex buffer[1], index buffer[2]
typedef unsigned int dmgui_material_t; // GLuint shader program
#define DMGUI_MATERIAL_INVALID 0xffffffff
typedef unsigned int dmgui_texture_t; // GLuint texture
#define DMGUI_TEXTURE_DEFAULT 0xffffffff
#define DMGUI_TEXTURE_INVALID 0
typedef void (*dmgui_mesh_layout_fn)(void*); // function to bind attributes to vao

typedef struct dmgui_viewport_t { float x, y, w, h; } dmgui_viewport_t;

typedef struct DmguiParamCreateRenderContext {
    intptr_t windowOrSurfaceOrId;
    dmgui_viewport_t viewport;
    dmgui_fcolor_t clearColor;
} DmguiParamCreateRenderContext;

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// 
// the remainder of this file is only available to the dmgui implementation
// treat everything after this comment as a private header
//
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#if defined(DMGUI_IMPLEMENTATION)

struct DmguiMesh {
    uint32_t vaoVboEbo[3];
    uint32_t vboSize;
    uint32_t vboCapacity;
    uint32_t eboSize;
    uint32_t eboCapacity;
};

struct DmguiRenderContext {
    intptr_t id;
    dmgui_viewport_t viewport;
    dmgui_fcolor_t clearColor;
    dmgui_material_t materialRect;
    dmgui_mesh_t meshRect;
    dmgui_material_t materialText;
    dmgui_mesh_t meshText;
    dmgui_texture_t textureDefault;
};

struct DmguiDraw {
    uint32_t verticesOffset;
    uint32_t verticesCount : 24;
    uint32_t vertexSize : 8;
    dmgui_index_t indicesOffset;
    dmgui_index_t indicesCount;

    dmgui_material_t material;
    dmgui_texture_t texture;
    dmgui_mesh_t mesh;
};

struct DmguiVertex {
    dmgui_floatv2_t pos;
    dmgui_floatv2_t uv;
    dmgui_color_t color;
};


// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// 
// DO NOT MODIFY ANYTHING BELOW THIS COMMENT
// 
// IF USING CUSTOM FUNCTIONS BELOW MUST BE IMPLEMENTED PER RENDER and INPUT API
// 
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

struct DmguiDraw;
struct DmguiVertex;

// required private internal - called by CreateGuiContext
int dmguiCreateRenderContext(const DmguiParamCreateRenderContext* in, size_t count, DmguiRenderContext* out);
// required private internal - called by DestroyGuiContext
int dmguiDestroyRenderContext(DmguiRenderContext* in, size_t count);
// required private internal - called by UpdateGuiContextBegin
int dmguiUpdateRenderContextBegin(DmguiRenderContext* ctx, dmgui_floatv2_t viewport, dmgui_floatv2_t canvas, DmguiDraw* prevDrawData, size_t prevDrawCount);
// required private internal - called by UpdateGuiContextEnd
int dmguiUpdateRenderContextEnd(DmguiRenderContext* ctx, dmgui_floatv2_t viewport, dmgui_floatv2_t canvas, dmgui_floatv4_t projMatrix[4], DmguiDraw* drawData, size_t drawCount, const uint8_t* vertexData, size_t vertexCount, const dmgui_index_t* indexData, size_t indexCount);

#endif