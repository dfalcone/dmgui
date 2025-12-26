#include "dmgui/dmgui.h"
#include "dmgui/dmgui_font.h"

#include <stdio.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_opengl.h>

SDL_Window* window = 0;
dmgui_context_t* gui = 0;
dmgui_floatv2_t viewportResolution = { 1920, 1080 };
dmgui_floatv2_t canvasResolution = { 1920, 1080 };
dmgui_floatv2_t helloWorldRectPosDelta = { 0.f, 0.f };

// font ids are a 0 based index
enum {
    FONT_DEFAULT
};

static void CreateDmgui() {
    DmguiParamCreateRenderContext render;
    render.windowOrSurfaceOrId = (intptr_t)window;
    render.viewport = (dmgui_viewport_t){ 0.f, 0.f, 1.f, 1.f }; // entire window surface
    render.clearColor = (dmgui_fcolor_t){ .2f, .2f, .2f, 1.f }; // light grey
    DmguiParamCreateInputContext input;
    input.consumeInputEvents = false;
    input.pumpEvents = true;
    gui = dmguiCreateContext(viewportResolution, canvasResolution, &render, &input);
    dmgui_font_t fontId = dmguiCreateFont("Ubuntu-Regular.ttf");
    SDL_assert(fontId == FONT_DEFAULT);
}

static void DestroyDmgui() {
    dmguiDestroyContext(gui);
}

static void OnHelloWorldRectDrag(dmgui_index_t idx, dmgui_floatv2_t posDelta)
{
    helloWorldRectPosDelta.x += posDelta.x;
    helloWorldRectPosDelta.y += posDelta.y;
}

static bool UpdateDmgui() {
    {   // in case user resized
        // for realtime resizing move to thread event filter
        // or put gui in separate thread from main
        int w,h;
        SDL_GetWindowSize(window, &w, &h);
        viewportResolution = (dmgui_floatv2_t){ (float)w, (float)h };
        dmguiSetViewportResolution(viewportResolution);
    }

    dmguiUpdateContextBegin();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            return false;
        case SDL_WINDOWEVENT:
            switch (event.window.event) {
            case SDL_WINDOWEVENT_CLOSE:
                return false;
            }
        }
    }

    { // hello world text on solid fill rect
        DmguiTextComponent text = {0};
        text.text = "Hello World";
        text.textColor = (dmgui_color_t){ 0, 0, 0, 255 };
        text.fontSize = 32;
        text.fontId = 0;

        DmguiComponent coms[] = {
            { .type = DMGUI_COMPONENT_FILL, .color = (dmgui_color_t){ 200, 100, 50, 255 } },
            { .type = DMGUI_COMPONENT_TEXT, .text = &text },
            { .type = DMGUI_COMPONENT_BIND_DRAG, .bindButton = 1, .bindCallbackDrag = OnHelloWorldRectDrag },
        };

        DmguiObject obj = { 0 };
        obj.id = (const dmgui_id_t*)"helloworld";
        obj.pos = (dmgui_floatv2_t){ 300.f + helloWorldRectPosDelta.x, 300.f + helloWorldRectPosDelta.y };
        obj.size = (dmgui_floatv2_t){ 300.f, 300.f };
        obj.componentCount = sizeof(coms) / sizeof(DmguiComponent);
        obj.components = coms;
        dmguiDrawObject(&obj);
    }

    dmguiUpdateContextEnd();

    return true;
}

int main()
{
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("Error: Unable to initialize SDL\n[%s]\n", SDL_GetError());
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window = SDL_CreateWindow("dmgui demo_sdl2_gl3",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        (int)viewportResolution.x, (int)viewportResolution.y,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Error: Unable to create SDL window\n[%s]\n", SDL_GetError());
        return -1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        printf("Error: Unable to create OpenGL context\n[%s]\n", SDL_GetError());
        return -1;
    }
    
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    CreateDmgui();

    while (UpdateDmgui()) {
        SDL_GL_SwapWindow(window);
    }

    DestroyDmgui();

    return 0;
}