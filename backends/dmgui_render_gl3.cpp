#define DMGUI_IMPLEMENTATION
#include "opengl/dmgui_opengl_loader.h"
#include "dmgui/dmgui_config.h"
#include "dmgui/dmgui.h"
//#include "dmgui_render_gl3.h"
#include "dmgui/dmgui_font.h"

#include <assert.h>
#include <stdio.h>

#if defined(__EMSCRIPTEN__)
#define GLSL_VERSION_PRECISION "#version 300 es\n" "precision mediump float;\n"
#else
#define GLSL_VERSION_PRECISION "#version 330 core\n"
#endif

// todo: add depth to pos
constexpr char c_shaderVertDefaultGlsl[] = GLSL_VERSION_PRECISION R"(
layout (location = 0) in vec2 Vert_Pos;
layout (location = 1) in vec2 Vert_UV;
layout (location = 2) in vec4 Vert_Color;
uniform Pc { mat3 uProj; } pc;
out vec4 Frag_Color;
out vec2 Frag_UV;
const float depthScale = -1e-5; // +: back-to-front, -: front-to-back
void main() {
    Frag_Color = Vert_Color;
    Frag_UV = Vert_UV;
    gl_Position = vec4(pc.uProj * vec3(Vert_Pos, 1), 1);
    gl_Position.y *= -1.0;
})";

constexpr char c_shaderFragDefaultGlsl[] = GLSL_VERSION_PRECISION R"(
in vec4 Frag_Color;
in vec2 Frag_UV;
uniform sampler2D sTexture;
out vec4 Out_Color;
void main() {
    Out_Color = Frag_Color * texture(sTexture, Frag_UV.st);
    Out_Color.a = 1.f;
})";

const char c_shaderVertTextGlsl[] = GLSL_VERSION_PRECISION R"(
layout(location = 0) in vec2 aPos;      // vertex position
layout(location = 1) in vec2 aUV;       // texcoord of glyph bbox
layout(location = 2) in vec4 aColor;
layout(location = 3) in uvec2 aSegData; // {offset, count}
layout(location = 4) in ivec4 aBounds;  // {boundsMinXY, boundsSizeZW}
uniform Pc { mat3 uProj; } pc;
out vec4 fragColor;
out vec2 fragUV;
flat out int fragOffset;
flat out int fragCount;
const float depthScale = -1e-5; // +: back-to-front, -: front-to-back
void main() {
    fragOffset = int(aSegData.x) * 2;
    fragCount = int(aSegData.y) * 2;
    fragColor = aColor;
    fragUV = vec2(aBounds.xy) + aUV * vec2(aBounds.zw);
    gl_Position = vec4(pc.uProj * vec3(aPos, 1), 1);
    gl_Position.y *= -1.0;
})";

const char c_shaderFragTextGlsl[] = GLSL_VERSION_PRECISION R"(
// 1D signed 16-bit RGBA texture with segment data
// each segment has 2 texels of data
// { px0, py0, px2, py2 }, { cpx, cpy, segCount, pad }
uniform isampler2D uSegmentsTex;
flat in int fragOffset; // starting segment index for this glyph
flat in int fragCount;  // total number of segments in this glyph
in vec2 fragUV;         // interpolated position inside glyph quad, in pixel space
in vec4 fragColor;
out vec4 outColor;
const int c_maxTexDim = 2048;
const int c_quadraticSteps = 4; // 4 is normal, 5 for slightly higher quality, or outline
const int c_raycastSteps = 8; // for fill
const float pxScale = 1.0 / 4096.0; // appox scale into pixel space for better precision

// Evaluate quadratic bezier point at t
vec2 bezierPoint(float t, vec2 a, vec2 b, vec2 c) {
    float u = 1.0 - t;
    return u*u*a + 2.0*u*t*b + t*t*c;
}

float bezierRayDistance(vec2 p, vec2 a, vec2 b, vec2 c, out int winding) {
    winding = 0;
    float minDist = 1e10;
    vec2 prev = a;
    for (int i = 1; i <= c_raycastSteps; ++i) {
        float t = float(i) / float(c_raycastSteps);
        vec2 pt = bezierPoint(t, a, b, c);
        // Ray crossing logic
        if ((prev.y > p.y) != (pt.y > p.y)) {
            float dx = pt.x - prev.x;
            float dy = pt.y - prev.y;
            float xint = prev.x + dx * (p.y - prev.y) / (dy + 1e-6);
            if (xint > p.x)
                winding += (pt.y > prev.y) ? 1 : -1;
        }
        // Distance to line segment
        vec2 segDir = pt - prev;
        vec2 toP = p - prev;
        float tProj = clamp(dot(toP, segDir) / dot(segDir, segDir), 0.0, 1.0);
        vec2 proj = prev + segDir * tProj;
        float dist = length(proj - p);
        minDist = min(minDist, dist);
        prev = pt;
    }
    return minDist;
}

ivec2 texelCoord(int i) { return ivec2(i % c_maxTexDim, i / c_maxTexDim); }

void main() {
    int winding = 0;
    float minDist = 1e20; // sdf
    vec2 uv = fragUV * pxScale;

    for (int i = 0; i < fragCount;) {
        ivec2 texel0 = texelCoord(fragOffset + i);
        ++i;
        ivec2 texel1 = texelCoord(fragOffset + i);
        ++i;
        ivec4 posInt = texelFetch(uSegmentsTex, texel0, 0);
        ivec4 cpInt = texelFetch(uSegmentsTex, texel1, 0);

        vec2 a = vec2(posInt.xy) * pxScale;
        vec2 c = vec2(posInt.zw) * pxScale;
        vec2 b = vec2(cpInt.xy) * pxScale;

        int rayWinding = 0;
        float rayDist = bezierRayDistance(uv, a, b, c, rayWinding);
        minDist = min(minDist, abs(rayDist));
        winding += rayWinding;
    }
    float signedDist = (winding & 1) != 0 ? -minDist : minDist;
    float antialiasing = fwidth(signedDist) * 0.5;
    float alpha = smoothstep(0.0 + antialiasing, 0.0 - antialiasing, signedDist);
    outColor = vec4(fragColor.rgb, fragColor.a * alpha);
}
)";

#define UBO_BIND 0
#define TEX_BIND 1

// will get a 4x4 projection matrix as input to UpdateGuiRenderContextEnd
// defining this type to match this implementation's shader which uses a 3x4 matrix
// note that in glsl that a mat3 inside a uniform buffer is 3 vec4s because of padding
typedef float proj_mat_t[3][4];

static GLuint s_ubo = 0;
static GLuint s_vaoVboEboRect[3] = {0};
static GLuint s_vaoVboEboText[3] = {0};
static dmgui_texture_t s_textureDefault = 0;

static dmgui_vector_t<DmguiMesh> s_meshes;

struct Program {
    GLuint program;
    GLuint shaderVert;
    GLuint shaderFrag;
};

static Program s_programRect;
static Program s_programText;

static GLuint CompileShader(GLenum type, const char** source, GLint sourceLen) {
    GLuint shader;
    shader = glCreateShader(type);
    glShaderSource(shader, 1, source, &sourceLen);
    glCompileShader(shader);
    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char msg[4096];
        glGetShaderInfoLog(shader, sizeof(msg), NULL, msg);
        printf("error: failed to compile %s shader\n%s",
            type == GL_VERTEX_SHADER ? "vertex" : "fragment",
            msg);
    }
    return shader;
}

static Program CreateProgram(const char* sourceVert, size_t sourceVertLen, const char* sourceFrag, size_t sourceFragLen) {
    int error; (void)error;
    Program p;
    p.shaderVert = CompileShader(GL_VERTEX_SHADER, &sourceVert, sourceVertLen);
    p.shaderFrag = CompileShader(GL_FRAGMENT_SHADER, &sourceFrag, sourceFragLen);
    assert(!(error = glGetError()) && p.shaderVert && p.shaderFrag && "error: failed to compile glsl shaders");
    p.program = glCreateProgram();
    glAttachShader(p.program, p.shaderVert);
    glAttachShader(p.program, p.shaderFrag);
    glLinkProgram(p.program);
    GLint status = GL_FALSE;
    glGetProgramiv(p.program, GL_LINK_STATUS, &status);
    if (!status) {
        char msg[4096];
        glGetProgramInfoLog(p.program, sizeof(msg), NULL, msg);
        printf("error: failed to link shader\n%s", msg);
    }
    assert(!(error = glGetError()) && status && "error: failed to link gl program");
    return p;
}

static void DestroyPipeline(GLuint program) {

    glDeleteProgram(program);
}

static void CreatePerFrameBuffers() {
        glGenBuffers(1, &s_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, s_ubo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(proj_mat_t), NULL, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, UBO_BIND, s_ubo);
        assert(!glGetError());
}

struct VertexAttribute {
    GLint componentCount;
    GLenum componentType;
    bool normalized;
    bool integer;
    uint8_t size;
};

static uint32_t CreateMesh(GLsizei vertexSize, GLuint attribCount, const VertexAttribute* attribs) {
    uint32_t idx;
    DmguiMesh mesh{};
    glGenVertexArrays(1, &mesh.vaoVboEbo[0]);
    glGenBuffers(2, &mesh.vaoVboEbo[1]);
    glBindVertexArray(mesh.vaoVboEbo[0]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vaoVboEbo[1]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vaoVboEbo[2]);
    uint8_t* attribOff = 0;
    for (GLuint i = 0; i < attribCount; ++i) {
        VertexAttribute attrib = attribs[i];
        glEnableVertexAttribArray(i);
        if (attrib.integer)
            glVertexAttribIPointer(i, attrib.componentCount, attrib.componentType, vertexSize, attribOff);
        else
            glVertexAttribPointer(i, attrib.componentCount, attrib.componentType, attrib.normalized, vertexSize, attribOff);
        attribOff += attrib.size;
    }
    assert(!glGetError() && vertexSize == (size_t)attribOff);

    glBufferData(GL_ARRAY_BUFFER, 1024*1024, NULL, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 1024*1024, NULL, GL_DYNAMIC_DRAW);
    idx = (uint32_t)s_meshes.size();
    s_meshes.push_back(mesh);
    return idx;
}

static void CreateDefaultTexture() {
    dmgui_color_t white = { 255, 255, 255, 255 };
    glGenTextures(1, &s_textureDefault);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_textureDefault);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &white);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    assert(!glGetError());
}

// called on
static int CreateSystem() {
    // load gl function pointers
    if (!dmguiLoadGl3()) {
        assert(0 && "failed to load opengl");
        return false;
    }

    // load shaders
    s_programRect = CreateProgram(
        c_shaderVertDefaultGlsl, sizeof(c_shaderVertDefaultGlsl),
        c_shaderFragDefaultGlsl, sizeof(c_shaderFragDefaultGlsl));
    assert(!glGetError());
    s_programText = CreateProgram(
        c_shaderVertTextGlsl, sizeof(c_shaderVertTextGlsl),
        c_shaderFragTextGlsl, sizeof(c_shaderFragTextGlsl));
    assert(!glGetError());

    // uniform bindings - GL4, GLES3 and WebGL2 support glsl bindings but GL3 does not
    GLuint uboIdx = glGetUniformBlockIndex(s_programRect.program, "Pc");
    assert(!glGetError() && uboIdx != GL_INVALID_INDEX);
    glUniformBlockBinding(s_programRect.program, uboIdx, 0);
    assert(!glGetError());
    GLuint sampler = glGetUniformLocation(s_programRect.program, "sTexture");
    assert(!glGetError() && sampler != GL_INVALID_VALUE );
    glUseProgram(s_programRect.program);
    glUniform1i(sampler, 0);
    assert(!glGetError());
    sampler = glGetUniformLocation(s_programText.program, "uSegmentsTex");
    assert(!glGetError() && sampler != GL_INVALID_VALUE);
    glUseProgram(s_programText.program);
    glUniform1i(sampler, 0);
    assert(!glGetError());

    // uniform buffor object - projection matrix
    CreatePerFrameBuffers();

    // vertex array objects - mesh
    VertexAttribute attribs[5];
    attribs[0] = { 2, GL_FLOAT, GL_FALSE, false, 8 }; // pos
    attribs[1] = { 2, GL_FLOAT, GL_FALSE, false, 8 }; // uv
    attribs[2] = { 4, GL_UNSIGNED_BYTE, GL_TRUE, false, 4 }; // color
    CreateMesh(sizeof(DmguiVertex), 3, attribs);
    attribs[3] = { 2, GL_UNSIGNED_INT, GL_FALSE, true, 8 }; // glyph segments
    attribs[4] = { 4, GL_SHORT, GL_FALSE, true, 8 }; // glyph bounds
    CreateMesh(sizeof(DmguiFontVertex), 5, attribs);

    // unbind vao/buffers
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // default white texture
    CreateDefaultTexture();
    return glGetError() == 0;
}

static void DestroySystem() {

}

int dmguiCreateRenderContext(const DmguiParamCreateRenderContext* in, size_t count,
    DmguiRenderContext* out
) {
    out->id = in->windowOrSurfaceOrId;
    out->viewport = in->viewport;
    out->clearColor = in->clearColor;
    if (count == 1) {
        CreateSystem();
    }
    // shader program is mapped to dmgui_material_t
    out->materialRect = s_programRect.program;
    //out->meshRect = (dmgui_mesh_t)s_vaoVboEboRect[0] << 32 | s_vaoVboEboRect[1];
    out->meshRect = 0;
    out->materialText = s_programText.program;
    //out->meshText = (dmgui_mesh_t)s_vaoVboEboText[0] << 32 | s_vaoVboEboText[1];
    out->meshText = 1;
    out->textureDefault = s_textureDefault;
    return 1;
}

int dmguiDestroyRenderContext(DmguiRenderContext* in, size_t count) {
    if (count == 0) {
        DestroySystem();
    }
    return 1;
}

int dmguiUpdateRenderContextBegin(DmguiRenderContext* ctx,
    dmgui_floatv2_t viewport, dmgui_floatv2_t canvas,
    DmguiDraw* prevDrawData, size_t prevDrawCount
) {
    // this would be where to queue recycle or destroy resources from prev frame
    // currently relying on opengl driver to handle this
    return 1;
}

//struct PackedBuffers {
//    std::vector<uint8_t> vertexData;
//    std::vector<uint32_t> indexData;
//};
//
//struct CombinedDraw {
//    PackedBuffers buffers;
//    DmguiDraw draws;
//};
//
//static std::unordered_map<dmgui_material_t, CombinedDraw> PackByShader(
//    DmguiDraw* draws, size_t drawsCount,
//    const uint8_t* globalVertexBuffer,
//    const dmgui_index_t* globalIndexBuffer
//) {
//    std::unordered_map<dmgui_material_t, CombinedDraw> result;
//
//    // key = (material << 32) | texture
//    std::unordered_map<uint64_t, CombinedDraw*> combinedMap;
//
//    CombinedDraw* combined = nullptr;
//    for (size_t i = 0; i < drawsCount; ++i) {
//        const* drawIn = &draws[i];
//        const dmgui_material_t material = drawIn->material;
//        if (!combined || combined->draws.material != material)
//            combined = &result[material];
//
//        DmguiDraw* draw = &draws[i];
//
//        PackedBuffers& out = combined->buffers;
//
//        uint32_t dstVertexOffset = static_cast<uint32_t>(out.vertexData.size());
//        uint32_t newBaseVertex = dstVertexOffset / draw.vertexSize;
//
//        // Copy vertex data
//        size_t vertexBytes = draw.verticesCount * draw.vertexSize;
//        out.vertexData.resize(dstVertexOffset + vertexBytes);
//        std::memcpy(
//            out.vertexData.data() + dstVertexOffset,
//            globalVertexBuffer + draw.verticesOffset,
//            vertexBytes
//        );
//
//        // Remap and copy indices
//        uint32_t newIndexOffset = static_cast<uint32_t>(out.indexData.size());
//        for (uint32_t j = 0; j < draw.indicesCount; ++j) {
//            uint32_t globalIndex = globalIndexBuffer[draw.indicesOffset + j];
//            uint32_t localIndex = (globalIndex - draw.verticesOffset) / draw.vertexSize;
//            out.indexData.push_back(newBaseVertex + localIndex);
//        }
//
//        // Create new merged draw entry
//        DmguiDraw packedDraw = draw;
//        packedDraw.verticesOffset = dstVertexOffset;
//        packedDraw.indicesOffset = newIndexOffset;
//
//        combined->draws.push_back(packedDraw);
//    }
//
//    return result;
//}

int dmguiUpdateRenderContextEnd(DmguiRenderContext* ctx,
    dmgui_floatv2_t viewport, dmgui_floatv2_t canvas,
    dmgui_floatv4_t projMatrix[4],
    DmguiDraw* drawData, size_t drawCount,
    const uint8_t* vertexData, size_t vertexDataSize,
    const dmgui_index_t* indexData, size_t indexCount
) {
    assert(!glGetError() && "dmguiUpdateRenderContextEnd - start");
    glDisable(GL_DEPTH_TEST);
    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL); // Or GL_LESS
    //glDisable(GL_STENCIL_TEST);

    glFrontFace(GL_CW);
    glDisable(GL_CULL_FACE);
    //glEnable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glBlendEquation(GL_FUNC_ADD);
    //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_SCISSOR_TEST);
    glViewport(0, 0, viewport.x, viewport.y);
    glScissor(0, 0, viewport.x, viewport.y);
    assert(!glGetError() && "glViewport glScissor");
    // only clear if provided alpha
    if (ctx->clearColor.a > 0.f) {
        glClearColor(ctx->clearColor.r, ctx->clearColor.g, ctx->clearColor.b, ctx->clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        assert(!glGetError() && "glClear");
    }
    else {
        glClear(GL_DEPTH_BUFFER_BIT);
        assert(!glGetError() && "glClear");
    }

    // update global shared uniforms
    glBindBuffer(GL_UNIFORM_BUFFER, s_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(proj_mat_t), projMatrix);
    assert(!glGetError() && "glBufferData");

    // only one texture slot is used for all shaders
    glActiveTexture(GL_TEXTURE0);

    constexpr GLenum glIndexType = sizeof(dmgui_index_t) == sizeof(uint16_t) ?
        GL_UNSIGNED_SHORT :
        GL_UNSIGNED_INT;

    dmgui_material_t material = DMGUI_MATERIAL_INVALID;
    dmgui_texture_t texture = DMGUI_TEXTURE_INVALID;

    const uint8_t* drawVertexData = vertexData;
    const dmgui_index_t* drawIndexData = indexData;
    size_t drawVertexCount = 0;
    size_t drawIndexCount = 0;
    size_t baseVertex = 0;
    size_t baseIndex = 0;
    for (uint32_t i = 0; i < drawCount; ++i) {
        bool isDrawCall = i == drawCount - 1;
        DmguiDraw draw = drawData[i];
        drawVertexCount += draw.verticesCount;
        drawIndexCount += draw.indicesCount;

        if (draw.material != material) {
            material = draw.material;
            glUseProgram(material);
            assert(!glGetError() && "glUseProgram");
            isDrawCall = true;
            baseVertex = 0;
            baseIndex = 0;
            drawVertexData = vertexData + draw.verticesOffset;
            drawIndexData = indexData + draw.indicesOffset;

            DmguiMesh* mesh = &s_meshes[draw.mesh];
            mesh->vboSize = 0;
            mesh->eboSize = 0;
        }

        if (draw.texture != texture) {
            texture = draw.texture;
            glBindTexture(GL_TEXTURE_2D, texture);
            assert(!glGetError() && "glBindTexture");
            isDrawCall = true;
        }

        if (isDrawCall) {

            const size_t drawVertexDataSize = drawVertexCount * draw.vertexSize;
            const size_t drawIndexDataSize = drawIndexCount * sizeof(dmgui_index_t);

            DmguiMesh mesh = s_meshes[draw.mesh];
            const GLuint vao = mesh.vaoVboEbo[0];
            const GLuint vbo = mesh.vaoVboEbo[1];
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            // ebo automatically bound with vao

            mesh.vboSize += drawVertexDataSize;
            mesh.eboSize += drawIndexDataSize;
            if (mesh.vboSize > mesh.vboCapacity || mesh.eboSize > mesh.eboCapacity) {
                // grow mesh buffers
                mesh.vboCapacity = mesh.vboSize;
                mesh.eboCapacity = mesh.eboSize;
                glBufferData(GL_ARRAY_BUFFER, mesh.vboSize, NULL, GL_DYNAMIC_DRAW);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.eboSize, NULL, GL_DYNAMIC_DRAW);
            }

            // transfer mesh buffers
            glBufferSubData(GL_ARRAY_BUFFER, baseVertex * draw.vertexSize, drawVertexDataSize, drawVertexData);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, baseIndex * draw.vertexSize, drawIndexDataSize, drawIndexData);

            // base offset into one mesh per shader program
            glDrawElementsBaseVertex(GL_TRIANGLES, drawIndexCount, glIndexType, (void*)0, baseVertex);

            drawVertexData += drawVertexCount * draw.vertexSize;
            drawIndexData += drawIndexCount;
            baseVertex += drawVertexCount;
            baseIndex += drawIndexCount;
            drawVertexCount = 0;
            drawIndexCount = 0;
        }
    }

    // unbind vao and vbo to not leak gl state
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    assert(!glGetError() && "dmguiUpdateRenderContextEnd - end");
    return 1;
}

// internal lib function
dmgui_texture_t dmguiCreateFontTexture(uint32_t w, uint32_t h, void* data, size_t dataSize) {
    assert(dataSize >= w * h * 8);
    dmgui_texture_t tex;
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16I, w, h, 0, GL_RGBA_INTEGER, GL_SHORT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    assert(!glGetError() && "dmguiCreateFontTexture");
    return tex;
}
