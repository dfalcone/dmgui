#pragma once

#include <assert.h>

// minimal subset of OpenGL local to DMGUI
// compatible with:
// - GL 3.3 (Windows, MacOS, Linux, FreeBSD)
// - GLES 2.2 (Emscripten, Android)

#define GL_VERSION_4_6 0
#define GL_VERSION_4_5 0
#define GL_VERSION_4_4 0
#define GL_VERSION_4_3 0
#define GL_VERSION_4_2 0
#define GL_VERSION_4_1 0
#define GL_VERSION_4_0 0

// ---------------
#if defined(__EMSCRIPTEN__)
#include <GL/GL.h>
#include <GL/glext.h>
static inline bool dmguiLoadGl3(){ return true; }
// ---------------
#else // PC
//#include "glcorearb.h"
//#include "glext.h"
//#include "glxext.h"

#define DMGUI_OPENGL_LOADER_FNPTRS
static bool _dmguiLoadGl3FunctionPointers();

extern "C" {

#if defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__) && !defined(__SCITECH_SNAP__)
#define APIENTRY __stdcall
#endif

#ifndef APIENTRY
#define APIENTRY
#endif

#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif

#ifndef GLAPIENTRY
#define GLAPIENTRY APIENTRY
#endif

#include "khrplatform.h"
typedef void GLvoid;
typedef unsigned int GLenum;
typedef char GLchar;
typedef khronos_float_t GLfloat;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLbitfield;
typedef double GLdouble;
typedef unsigned int GLuint;
typedef unsigned char GLboolean;
typedef khronos_uint8_t GLubyte;
typedef khronos_ssize_t GLsizeiptr;
typedef khronos_intptr_t GLintptr;
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_STENCIL_BUFFER_BIT             0x00000400
#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_FALSE                          0
#define GL_TRUE                           1
#define GL_POINTS                         0x0000
#define GL_LINES                          0x0001
#define GL_LINE_LOOP                      0x0002
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_TRIANGLE_FAN                   0x0006
#define GL_QUADS                          0x0007
#define GL_NEVER                          0x0200
#define GL_LESS                           0x0201
#define GL_EQUAL                          0x0202
#define GL_LEQUAL                         0x0203
#define GL_GREATER                        0x0204
#define GL_NOTEQUAL                       0x0205
#define GL_GEQUAL                         0x0206
#define GL_ALWAYS                         0x0207
#define GL_ZERO                           0
#define GL_ONE                            1
#define GL_SRC_COLOR                      0x0300
#define GL_ONE_MINUS_SRC_COLOR            0x0301
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_DST_ALPHA                      0x0304
#define GL_ONE_MINUS_DST_ALPHA            0x0305
#define GL_DST_COLOR                      0x0306
#define GL_ONE_MINUS_DST_COLOR            0x0307
#define GL_SRC_ALPHA_SATURATE             0x0308
#define GL_NONE                           0
#define GL_FRONT_LEFT                     0x0400
#define GL_FRONT_RIGHT                    0x0401
#define GL_BACK_LEFT                      0x0402
#define GL_BACK_RIGHT                     0x0403
#define GL_FRONT                          0x0404
#define GL_BACK                           0x0405
#define GL_LEFT                           0x0406
#define GL_RIGHT                          0x0407
#define GL_FRONT_AND_BACK                 0x0408
#define GL_NO_ERROR                       0
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_OUT_OF_MEMORY                  0x0505
#define GL_CW                             0x0900
#define GL_CCW                            0x0901
#define GL_POINT_SIZE                     0x0B11
#define GL_POINT_SIZE_RANGE               0x0B12
#define GL_POINT_SIZE_GRANULARITY         0x0B13
#define GL_LINE_SMOOTH                    0x0B20
#define GL_LINE_WIDTH                     0x0B21
#define GL_LINE_WIDTH_RANGE               0x0B22
#define GL_LINE_WIDTH_GRANULARITY         0x0B23
#define GL_POLYGON_MODE                   0x0B40
#define GL_POLYGON_SMOOTH                 0x0B41
#define GL_CULL_FACE                      0x0B44
#define GL_CULL_FACE_MODE                 0x0B45
#define GL_FRONT_FACE                     0x0B46
#define GL_DEPTH_RANGE                    0x0B70
#define GL_DEPTH_TEST                     0x0B71
#define GL_DEPTH_WRITEMASK                0x0B72
#define GL_DEPTH_CLEAR_VALUE              0x0B73
#define GL_DEPTH_FUNC                     0x0B74
#define GL_STENCIL_TEST                   0x0B90
#define GL_STENCIL_CLEAR_VALUE            0x0B91
#define GL_STENCIL_FUNC                   0x0B92
#define GL_STENCIL_VALUE_MASK             0x0B93
#define GL_STENCIL_FAIL                   0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL        0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS        0x0B96
#define GL_STENCIL_REF                    0x0B97
#define GL_STENCIL_WRITEMASK              0x0B98
#define GL_VIEWPORT                       0x0BA2
#define GL_DITHER                         0x0BD0
#define GL_BLEND_DST                      0x0BE0
#define GL_BLEND_SRC                      0x0BE1
#define GL_BLEND                          0x0BE2
#define GL_LOGIC_OP_MODE                  0x0BF0
#define GL_DRAW_BUFFER                    0x0C01
#define GL_READ_BUFFER                    0x0C02
#define GL_SCISSOR_BOX                    0x0C10
#define GL_SCISSOR_TEST                   0x0C11
#define GL_COLOR_CLEAR_VALUE              0x0C22
#define GL_COLOR_WRITEMASK                0x0C23
#define GL_DOUBLEBUFFER                   0x0C32
#define GL_STEREO                         0x0C33
#define GL_LINE_SMOOTH_HINT               0x0C52
#define GL_POLYGON_SMOOTH_HINT            0x0C53
#define GL_UNPACK_SWAP_BYTES              0x0CF0
#define GL_UNPACK_LSB_FIRST               0x0CF1
#define GL_UNPACK_ROW_LENGTH              0x0CF2
#define GL_UNPACK_SKIP_ROWS               0x0CF3
#define GL_UNPACK_SKIP_PIXELS             0x0CF4
#define GL_UNPACK_ALIGNMENT               0x0CF5
#define GL_PACK_SWAP_BYTES                0x0D00
#define GL_PACK_LSB_FIRST                 0x0D01
#define GL_PACK_ROW_LENGTH                0x0D02
#define GL_PACK_SKIP_ROWS                 0x0D03
#define GL_PACK_SKIP_PIXELS               0x0D04
#define GL_PACK_ALIGNMENT                 0x0D05
#define GL_MAX_TEXTURE_SIZE               0x0D33
#define GL_MAX_VIEWPORT_DIMS              0x0D3A
#define GL_SUBPIXEL_BITS                  0x0D50
#define GL_TEXTURE_1D                     0x0DE0
#define GL_TEXTURE_2D                     0x0DE1
#define GL_TEXTURE_WIDTH                  0x1000
#define GL_TEXTURE_HEIGHT                 0x1001
#define GL_TEXTURE_BORDER_COLOR           0x1004
#define GL_DONT_CARE                      0x1100
#define GL_FASTEST                        0x1101
#define GL_NICEST                         0x1102
#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_STACK_OVERFLOW                 0x0503
#define GL_STACK_UNDERFLOW                0x0504
#define GL_CLEAR                          0x1500
#define GL_AND                            0x1501
#define GL_AND_REVERSE                    0x1502
#define GL_COPY                           0x1503
#define GL_AND_INVERTED                   0x1504
#define GL_NOOP                           0x1505
#define GL_XOR                            0x1506
#define GL_OR                             0x1507
#define GL_NOR                            0x1508
#define GL_EQUIV                          0x1509
#define GL_INVERT                         0x150A
#define GL_OR_REVERSE                     0x150B
#define GL_COPY_INVERTED                  0x150C
#define GL_OR_INVERTED                    0x150D
#define GL_NAND                           0x150E
#define GL_SET                            0x150F
#define GL_TEXTURE                        0x1702
#define GL_COLOR                          0x1800
#define GL_DEPTH                          0x1801
#define GL_STENCIL                        0x1802
#define GL_STENCIL_INDEX                  0x1901
#define GL_DEPTH_COMPONENT                0x1902
#define GL_RED                            0x1903
#define GL_GREEN                          0x1904
#define GL_BLUE                           0x1905
#define GL_ALPHA                          0x1906
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_POINT                          0x1B00
#define GL_LINE                           0x1B01
#define GL_FILL                           0x1B02
#define GL_KEEP                           0x1E00
#define GL_REPLACE                        0x1E01
#define GL_INCR                           0x1E02
#define GL_DECR                           0x1E03
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601
#define GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GL_LINEAR_MIPMAP_NEAREST          0x2701
#define GL_NEAREST_MIPMAP_LINEAR          0x2702
#define GL_LINEAR_MIPMAP_LINEAR           0x2703
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803
#define GL_REPEAT                         0x2901

#define GL_RGBA8                          0x8058
#define GL_RGBA16                         0x805B
#define GL_RGBA16I                        0x8D88
#define GL_RGBA_INTEGER                   0x8D99
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_UNIFORM_BUFFER                 0x8A11
#define GL_TEXTURE0                       0x84C0
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_BUFFER_SIZE                    0x8764
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_ARRAY_BUFFER_BINDING           0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING   0x8895
#define GL_STREAM_DRAW                    0x88E0
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8

#define GL_INVALID_INDEX                  0xFFFFFFFFu

typedef GLuint (APIENTRYP PFNGLCREATESHADERPROC)(GLenum type);
typedef void (APIENTRYP PFNGLDELETESHADERPROC) (GLuint shader);
typedef void (APIENTRYP PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void (APIENTRYP PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef void (APIENTRYP PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (APIENTRYP PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef GLuint (APIENTRYP PFNGLCREATEPROGRAMPROC) (void);
typedef void (APIENTRYP PFNGLDELETEPROGRAMPROC) (GLuint program);
typedef void (APIENTRYP PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void (APIENTRYP PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef GLint (APIENTRYP PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar *name);
typedef GLuint (APIENTRYP PFNGLGETUNIFORMBLOCKINDEXPROC) (GLuint program, const GLchar *uniformBlockName);
typedef void (APIENTRYP PFNGLUNIFORMBLOCKBINDINGPROC) (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
typedef void (APIENTRYP PFNGLUNIFORM1IPROC) (GLint location, GLint v0);

typedef void (APIENTRYP PFNGLGETINTEGERVPROC) (GLenum pname, GLint *data);
typedef void (APIENTRYP PFNGLGETBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);

typedef void (APIENTRYP PFNGLGENTEXTURESPROC) (GLsizei n, GLuint *textures);
typedef void (APIENTRYP PFNGLTEXIMAGE2DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void (APIENTRYP PFNGLTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
typedef void (APIENTRYP PFNGLTEXPARAMETERIPROC) (GLenum target, GLenum pname, GLint param);

typedef void (APIENTRYP PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRYP PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRYP PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void (APIENTRYP PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
typedef void (APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (APIENTRYP PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void (APIENTRYP PFNGLVERTEXATTRIBIPOINTERPROC) (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
typedef void (APIENTRYP PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRYP PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
typedef void (APIENTRYP PFNGLBINDVERTEXARRAYPROC) (GLuint array);
typedef void (APIENTRYP PFNGLDELETEVERTEXARRAYSPROC) (GLsizei n, const GLuint *arrays);
typedef void (APIENTRYP PFNGLGENERATEMIPMAPPROC) (GLenum target);
typedef void (APIENTRYP PFNGLDISABLEPROC) (GLenum cap);
typedef void (APIENTRYP PFNGLENABLEPROC) (GLenum cap);
typedef void (APIENTRYP PFNGLDEPTHFUNCPROC) (GLenum func);
typedef void (APIENTRYP PFNGLVIEWPORTPROC) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLSCISSORPROC) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLCLEARCOLORPROC) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (APIENTRYP PFNGLCLEARPROC) (GLbitfield mask);
typedef void (APIENTRYP PFNGLFRONTFACEPROC) (GLenum mode);
typedef void (APIENTRYP PFNGLBLENDFUNCPROC) (GLenum sfactor, GLenum dfactor);
//typedef void (APIENTRYP PFNGLBLENDFUNCSEPARATEPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void (APIENTRYP PFNGLUSEPROGRAMPROC) (GLuint program);
typedef void (APIENTRYP PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);
typedef void (APIENTRYP PFNGLDRAWELEMENTSPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices);
typedef void (APIENTRYP PFNGLDRAWELEMENTSBASEVERTEXPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);

typedef void (APIENTRYP PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef void (APIENTRYP PFNGLBINDTEXTUREPROC) (GLenum target, GLuint texture);
typedef void (APIENTRYP PFNGLBINDBUFFERBASEPROC) (GLenum target, GLuint index, GLuint buffer);
typedef GLenum (APIENTRYP PFNGLGETERRORPROC) (void);
} // extern "C"

//namespace {
struct DmguiGl {
    alignas(64)
    // once
    PFNGLCREATESHADERPROC         glCreateShader         = 0;
    PFNGLDELETESHADERPROC         glDeleteShader         = 0;
    PFNGLSHADERSOURCEPROC         glShaderSource         = 0;
    PFNGLCOMPILESHADERPROC        glCompileShader        = 0;
    PFNGLGETSHADERIVPROC          glGetShaderiv          = 0;
    PFNGLGETSHADERINFOLOGPROC     glGetShaderInfoLog     = 0;
    PFNGLGETPROGRAMIVPROC         glGetProgramiv         = 0;
    PFNGLGETPROGRAMINFOLOGPROC    glGetProgramInfoLog    = 0;
    PFNGLCREATEPROGRAMPROC        glCreateProgram        = 0;
    PFNGLDELETEPROGRAMPROC        glDeleteProgram        = 0;
    PFNGLLINKPROGRAMPROC          glLinkProgram          = 0;
    PFNGLATTACHSHADERPROC         glAttachShader         = 0;
    PFNGLGETUNIFORMLOCATIONPROC   glGetUniformLocation   = 0;
    PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex = 0;
    PFNGLUNIFORMBLOCKBINDINGPROC  glUniformBlockBinding  = 0;
    PFNGLUNIFORM1IPROC            glUniform1i            = 0;
    // per draw

    PFNGLGETINTEGERVPROC          glGetIntegerv          = 0;
    PFNGLGETBUFFERPARAMETERIVPROC glGetBufferParameteriv = 0;

    PFNGLGENTEXTURESPROC          glGenTextures          = 0;
    PFNGLTEXIMAGE2DPROC           glTexImage2D           = 0;
    PFNGLTEXSUBIMAGE2DPROC        glTexSubImage2D        = 0;
    PFNGLTEXPARAMETERIPROC        glTexParameteri        = 0;
    PFNGLGENBUFFERSPROC           glGenBuffers           = 0;
    PFNGLBINDBUFFERPROC           glBindBuffer           = 0;
    PFNGLBUFFERDATAPROC           glBufferData           = 0;
    PFNGLBUFFERSUBDATAPROC        glBufferSubData        = 0;
    PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = 0;
    PFNGLVERTEXATTRIBPOINTERPROC  glVertexAttribPointer  = 0;
    PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer = 0;
    PFNGLDELETEBUFFERSPROC        glDeleteBuffers        = 0;
    PFNGLGENVERTEXARRAYSPROC      glGenVertexArrays      = 0;
    PFNGLBINDVERTEXARRAYPROC      glBindVertexArray      = 0;
    PFNGLDELETEVERTEXARRAYSPROC   glDeleteVertexArrays   = 0;
    PFNGLGENERATEMIPMAPPROC       glGenerateMipmap       = 0;
    PFNGLDISABLEPROC              glDisable              = 0;
    PFNGLENABLEPROC               glEnable               = 0;
    PFNGLDEPTHFUNCPROC            glDepthFunc            = 0;
    PFNGLVIEWPORTPROC             glViewport             = 0;
    PFNGLSCISSORPROC              glScissor              = 0;
    PFNGLCLEARCOLORPROC           glClearColor           = 0;
    PFNGLCLEARPROC                glClear                = 0;
    PFNGLFRONTFACEPROC            glFrontFace            = 0;
    PFNGLBLENDFUNCPROC            glBlendFunc            = 0;
    //PFNGLBLENDFUNCSEPARATEPROC    glBlendFuncSeparate    = 0;
    PFNGLUSEPROGRAMPROC           glUseProgram           = 0;
    PFNGLUNIFORM1FPROC            glUniform1f            = 0;
    PFNGLDRAWELEMENTSPROC         glDrawElements         = 0;
    PFNGLDRAWELEMENTSBASEVERTEXPROC glDrawElementsBaseVertex = 0;
    // frequent
    PFNGLACTIVETEXTUREPROC        glActiveTexture        = 0;
    PFNGLBINDTEXTUREPROC          glBindTexture          = 0;
    PFNGLBINDBUFFERBASEPROC       glBindBufferBase       = 0;
    PFNGLGETERRORPROC             glGetError             = 0;

};
static DmguiGl _dmguiGl;
#endif // PC
// ---------------

// ---------------
// Windows
#if defined(_WIN32)
extern "C" {
typedef void* HMODULE;
typedef const char* LPCSTR;
typedef void* FARPROC;
typedef void (*PROC)();

#if !defined(_MSC_VER) && !defined(__stdcall)
#define __stdcall __attribute__((stdcall))
#endif

__declspec(dllimport) HMODULE __stdcall LoadLibraryA(LPCSTR);
__declspec(dllimport) FARPROC __stdcall GetProcAddress(HMODULE, LPCSTR);
typedef PROC (__stdcall* PFN_GLGETPROCADDRESS)(LPCSTR name);
}

static bool s_glFuncsloaded = 0;
static void* s_glModule = 0;
static PFN_GLGETPROCADDRESS wglGetProcAddress;
static inline PROC glGetProcAddress(const char* name) {
    PROC p = wglGetProcAddress(name);
    if (!p) p = (PROC)GetProcAddress(s_glModule, name);
    return p;
}
static bool dmguiLoadGl3() {
    if (s_glFuncsloaded) return true;
    s_glModule = LoadLibraryA("opengl32.dll");
    if (!s_glModule) return false;
    wglGetProcAddress = (PFN_GLGETPROCADDRESS)GetProcAddress(s_glModule, "wglGetProcAddress");
    if (!wglGetProcAddress) return false;
    s_glFuncsloaded = _dmguiLoadGl3FunctionPointers();
    return s_glFuncsloaded;
}
// ---------------
// MacOS
#elif defined(__APPLE__)
#include <dlfcn.h>
static void* glModule;
static inline void* glGetProcAddress(const char* name) { return dlsym(glModule, name); }
static bool dmguiLoadGl3() {
    bool loaded =
        (glModule = dlopen("/System/Library/Frameworks/OpenGL.framework/OpenGL", RTLD_LAZY | RTLD_LOCAL | RTLD_NOLOAD)) ? true :
        (glModule = dlopen("/System/Library/Frameworks/OpenGL.framework/OpenGL", RTLD_LAZY | RTLD_LOCAL)) ? true :
        false;
    if (!loaded) return false;
    return _dmguiLoadGl3FunctionPointers();
}
// ---------------
// Linux, Unix variants
#elif defined(__linux__) || defined(__unix__)
#include <dlfcn.h>
static void* glModule;
typedef void* (APIENTRYP PFNGLXGETPROCADDRESSPROC_PRIVATE)(const char*);
static PFNGLXGETPROCADDRESSPROC_PRIVATE glxGetProcAddress;
static inline void* glGetProcAddress(const char* name) {
   void* ret = glxGetProcAddress(name);
   return ret ? ret : dlsym(glModule, name);
}
static bool dmguiLoadGl3() {
    bool loaded =
        (glModule = dlopen("libGL.so.1", RTLD_LAZY | RTLD_LOCAL | RTLD_NOLOAD)) ? true :
        (glModule = dlopen("libGLX.so.0", RTLD_LAZY | RTLD_LOCAL | RTLD_NOLOAD)) ? true :
        (glModule = dlopen("libEGL.so.1", RTLD_LAZY | RTLD_LOCAL | RTLD_NOLOAD)) ? true :
        (glModule = dlopen("libGL.so.1", RTLD_LAZY | RTLD_LOCAL)) ? true :
        (glModule = dlopen("libGLX.so.0", RTLD_LAZY | RTLD_LOCAL)) ? true :
        (glModule = dlopen("libEGL.so.1", RTLD_LAZY | RTLD_LOCAL)) ? true :
        false;
    assert(glModule && "failed to load libGL.so or equivalent");
    if (!loaded) return false;
    glxGetProcAddress = (PFNGLXGETPROCADDRESSPROC_PRIVATE)dlsym(glModule, "glXGetProcAddressARB");
    assert(glxGetProcAddress && "failed  to load glxGetProcAddress");
    return _dmguiLoadGl3FunctionPointers();
}
#elif defined(__ANDROID__)
#include <EGL/egl.h>
static inline void* glGetProcAddress(const char* name) { return (void*)eglGetProcAddress(name); }
static bool dmguiLoadGl3() {
    return _dmguiLoadGl3FunctionPointers();
}
#else
opengl platform not_implemented
#endif

#if defined(DMGUI_OPENGL_LOADER_FNPTRS)
static bool _dmguiLoadGl3FunctionPointers() {
    size_t failed = 0;
    failed |= (size_t)(_dmguiGl.glCreateShader         = (PFNGLCREATESHADERPROC        )glGetProcAddress("glCreateShader"        )) == 0;
    assert(!failed && "gl func");
    failed |= (size_t)(_dmguiGl.glDeleteShader         = (PFNGLDELETESHADERPROC        )glGetProcAddress("glDeleteShader"        )) == 0;
    failed |= (size_t)(_dmguiGl.glShaderSource         = (PFNGLSHADERSOURCEPROC        )glGetProcAddress("glShaderSource"        )) == 0;
    failed |= (size_t)(_dmguiGl.glCompileShader        = (PFNGLCOMPILESHADERPROC       )glGetProcAddress("glCompileShader"       )) == 0;
    failed |= (size_t)(_dmguiGl.glGetShaderiv          = (PFNGLGETSHADERIVPROC         )glGetProcAddress("glGetShaderiv"         )) == 0;
    failed |= (size_t)(_dmguiGl.glGetShaderInfoLog     = (PFNGLGETSHADERINFOLOGPROC    )glGetProcAddress("glGetShaderInfoLog"    )) == 0;
    failed |= (size_t)(_dmguiGl.glGetProgramiv         = (PFNGLGETPROGRAMIVPROC        )glGetProcAddress("glGetProgramiv"        )) == 0;
    failed |= (size_t)(_dmguiGl.glGetProgramInfoLog    = (PFNGLGETPROGRAMINFOLOGPROC   )glGetProcAddress("glGetProgramInfoLog"   )) == 0;
    failed |= (size_t)(_dmguiGl.glCreateProgram        = (PFNGLCREATEPROGRAMPROC       )glGetProcAddress("glCreateProgram"       )) == 0;
    failed |= (size_t)(_dmguiGl.glDeleteProgram        = (PFNGLDELETEPROGRAMPROC       )glGetProcAddress("glDeleteProgram"       )) == 0;
    failed |= (size_t)(_dmguiGl.glLinkProgram          = (PFNGLLINKPROGRAMPROC         )glGetProcAddress("glLinkProgram"         )) == 0;
    failed |= (size_t)(_dmguiGl.glAttachShader         = (PFNGLATTACHSHADERPROC        )glGetProcAddress("glAttachShader"        )) == 0;
    failed |= (size_t)(_dmguiGl.glGetUniformLocation   = (PFNGLGETUNIFORMLOCATIONPROC  )glGetProcAddress("glGetUniformLocation"  )) == 0;
    failed |= (size_t)(_dmguiGl.glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC)glGetProcAddress("glGetUniformBlockIndex")) == 0;
    failed |= (size_t)(_dmguiGl.glUniformBlockBinding  = (PFNGLUNIFORMBLOCKBINDINGPROC )glGetProcAddress("glUniformBlockBinding" )) == 0;
    failed |= (size_t)(_dmguiGl.glUniform1i            = (PFNGLUNIFORM1IPROC           )glGetProcAddress("glUniform1i"           )) == 0;

    failed |= (size_t)(_dmguiGl.glGetIntegerv          = (PFNGLGETINTEGERVPROC         )glGetProcAddress("glGetIntegerv"         )) == 0;
    failed |= (size_t)(_dmguiGl.glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC)glGetProcAddress("glGetBufferParameteriv")) == 0;

    failed |= (size_t)(_dmguiGl.glGenTextures          = (PFNGLGENTEXTURESPROC         )glGetProcAddress("glGenTextures"         )) == 0;
    failed |= (size_t)(_dmguiGl.glTexImage2D           = (PFNGLTEXIMAGE2DPROC          )glGetProcAddress("glTexImage2D"          )) == 0;
    failed |= (size_t)(_dmguiGl.glTexSubImage2D        = (PFNGLTEXSUBIMAGE2DPROC       )glGetProcAddress("glTexSubImage2D"       )) == 0;
    failed |= (size_t)(_dmguiGl.glTexParameteri        = (PFNGLTEXPARAMETERIPROC       )glGetProcAddress("glTexParameteri"       )) == 0;

    failed |= (size_t)(_dmguiGl.glGenBuffers           = (PFNGLGENBUFFERSPROC          )glGetProcAddress("glGenBuffers"          )) == 0;
    failed |= (size_t)(_dmguiGl.glBindBuffer           = (PFNGLBINDBUFFERPROC          )glGetProcAddress("glBindBuffer"          )) == 0;
    failed |= (size_t)(_dmguiGl.glBufferData           = (PFNGLBUFFERDATAPROC          )glGetProcAddress("glBufferData"          )) == 0;
    failed |= (size_t)(_dmguiGl.glBufferSubData        = (PFNGLBUFFERSUBDATAPROC       )glGetProcAddress("glBufferSubData"       )) == 0;
    failed |= (size_t)(_dmguiGl.glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)glGetProcAddress("glEnableVertexAttribArray")) == 0;
    failed |= (size_t)(_dmguiGl.glVertexAttribPointer  = (PFNGLVERTEXATTRIBPOINTERPROC )glGetProcAddress("glVertexAttribPointer" )) == 0;
    failed |= (size_t)(_dmguiGl.glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC)glGetProcAddress("glVertexAttribIPointer")) == 0;
    failed |= (size_t)(_dmguiGl.glDeleteBuffers        = (PFNGLDELETEBUFFERSPROC       )glGetProcAddress("glDeleteBuffers"       )) == 0;
    failed |= (size_t)(_dmguiGl.glGenVertexArrays      = (PFNGLGENVERTEXARRAYSPROC     )glGetProcAddress("glGenVertexArrays"     )) == 0;
    failed |= (size_t)(_dmguiGl.glBindVertexArray      = (PFNGLBINDVERTEXARRAYPROC     )glGetProcAddress("glBindVertexArray"     )) == 0;
    failed |= (size_t)(_dmguiGl.glDeleteVertexArrays   = (PFNGLDELETEVERTEXARRAYSPROC  )glGetProcAddress("glDeleteVertexArrays"  )) == 0;
    failed |= (size_t)(_dmguiGl.glGenerateMipmap       = (PFNGLGENERATEMIPMAPPROC      )glGetProcAddress("glGenerateMipmap"      )) == 0;
    failed |= (size_t)(_dmguiGl.glDisable              = (PFNGLDISABLEPROC             )glGetProcAddress("glDisable"             )) == 0;
    failed |= (size_t)(_dmguiGl.glEnable               = (PFNGLENABLEPROC              )glGetProcAddress("glEnable"              )) == 0;
    failed |= (size_t)(_dmguiGl.glDepthFunc            = (PFNGLDEPTHFUNCPROC           )glGetProcAddress("glDepthFunc"           )) == 0;
    failed |= (size_t)(_dmguiGl.glViewport             = (PFNGLVIEWPORTPROC            )glGetProcAddress("glViewport"            )) == 0;
    failed |= (size_t)(_dmguiGl.glScissor              = (PFNGLSCISSORPROC             )glGetProcAddress("glScissor"             )) == 0;
    failed |= (size_t)(_dmguiGl.glClearColor           = (PFNGLCLEARCOLORPROC          )glGetProcAddress("glClearColor"          )) == 0;
    failed |= (size_t)(_dmguiGl.glClear                = (PFNGLCLEARPROC               )glGetProcAddress("glClear"               )) == 0;
    failed |= (size_t)(_dmguiGl.glFrontFace            = (PFNGLFRONTFACEPROC           )glGetProcAddress("glFrontFace"           )) == 0;
    failed |= (size_t)(_dmguiGl.glBlendFunc            = (PFNGLBLENDFUNCPROC           )glGetProcAddress("glBlendFunc"           )) == 0;
    failed |= (size_t)(_dmguiGl.glUseProgram           = (PFNGLUSEPROGRAMPROC          )glGetProcAddress("glUseProgram"          )) == 0;
    failed |= (size_t)(_dmguiGl.glUniform1f            = (PFNGLUNIFORM1FPROC           )glGetProcAddress("glUniform1f"           )) == 0;
    failed |= (size_t)(_dmguiGl.glDrawElements         = (PFNGLDRAWELEMENTSPROC        )glGetProcAddress("glDrawElements"        )) == 0;
    failed |= (size_t)(_dmguiGl.glDrawElementsBaseVertex = (PFNGLDRAWELEMENTSBASEVERTEXPROC)glGetProcAddress("glDrawElementsBaseVertex")) == 0;

    failed |= (size_t)(_dmguiGl.glActiveTexture        = (PFNGLACTIVETEXTUREPROC       )glGetProcAddress("glActiveTexture"       )) == 0;
    failed |= (size_t)(_dmguiGl.glBindTexture          = (PFNGLBINDTEXTUREPROC         )glGetProcAddress("glBindTexture"         )) == 0;
    failed |= (size_t)(_dmguiGl.glBindBufferBase       = (PFNGLBINDBUFFERBASEPROC      )glGetProcAddress("glBindBufferBase"      )) == 0;
    failed |= (size_t)(_dmguiGl.glGetError             = (PFNGLGETERRORPROC            )glGetProcAddress("glGetError"            )) == 0;
    return !failed;
}

#define glCreateShader              _dmguiGl.glCreateShader
#define glDeleteShader              _dmguiGl.glDeleteShader
#define glShaderSource              _dmguiGl.glShaderSource
#define glCompileShader             _dmguiGl.glCompileShader
#define glGetShaderiv               _dmguiGl.glGetShaderiv
#define glGetShaderInfoLog          _dmguiGl.glGetShaderInfoLog
#define glGetProgramiv              _dmguiGl.glGetProgramiv
#define glGetProgramInfoLog         _dmguiGl.glGetProgramInfoLog
#define glCreateProgram             _dmguiGl.glCreateProgram
#define glDeleteProgram             _dmguiGl.glDeleteProgram
#define glLinkProgram               _dmguiGl.glLinkProgram
#define glAttachShader              _dmguiGl.glAttachShader
#define glGetUniformLocation        _dmguiGl.glGetUniformLocation
#define glGetUniformBlockIndex      _dmguiGl.glGetUniformBlockIndex
#define glUniformBlockBinding       _dmguiGl.glUniformBlockBinding
#define glUniform1i                 _dmguiGl.glUniform1i
#define glGetIntegerv               _dmguiGl.glGetIntegerv
#define glGetBufferParameteriv      _dmguiGl.glGetBufferParameteriv
#define glGenTextures               _dmguiGl.glGenTextures
#define glTexImage2D                _dmguiGl.glTexImage2D
#define glTexSubImage2D             _dmguiGl.glTexSubImage2D
#define glTexParameteri             _dmguiGl.glTexParameteri
#define glGenBuffers                _dmguiGl.glGenBuffers
#define glBindBuffer                _dmguiGl.glBindBuffer
#define glBufferData                _dmguiGl.glBufferData
#define glBufferSubData             _dmguiGl.glBufferSubData
#define glEnableVertexAttribArray   _dmguiGl.glEnableVertexAttribArray
#define glVertexAttribPointer       _dmguiGl.glVertexAttribPointer
#define glVertexAttribIPointer      _dmguiGl.glVertexAttribIPointer
#define glDeleteBuffers             _dmguiGl.glDeleteBuffers
#define glGenVertexArrays           _dmguiGl.glGenVertexArrays
#define glBindVertexArray           _dmguiGl.glBindVertexArray
#define glDeleteVertexArrays        _dmguiGl.glDeleteVertexArrays
#define glGenerateMipmap            _dmguiGl.glGenerateMipmap
#define glDisable                   _dmguiGl.glDisable
#define glEnable                    _dmguiGl.glEnable
#define glDepthFunc                 _dmguiGl.glDepthFunc
#define glViewport                  _dmguiGl.glViewport
#define glScissor                   _dmguiGl.glScissor
#define glClearColor                _dmguiGl.glClearColor
#define glClear                     _dmguiGl.glClear
#define glFrontFace                 _dmguiGl.glFrontFace
#define glBlendFunc                 _dmguiGl.glBlendFunc
#define glBlendFuncSeparate         _dmguiGl.glBlendFuncSeparate
#define glUseProgram                _dmguiGl.glUseProgram
#define glUniform1f                 _dmguiGl.glUniform1f
#define glDrawElements              _dmguiGl.glDrawElements
#define glDrawElementsBaseVertex    _dmguiGl.glDrawElementsBaseVertex
#define glActiveTexture             _dmguiGl.glActiveTexture
#define glBindTexture               _dmguiGl.glBindTexture
#define glBindBufferBase            _dmguiGl.glBindBufferBase
#define glGetError                  _dmguiGl.glGetError

#endif // DMGUI_OPENGL_LOADER_FNPTRS
// ---------------
