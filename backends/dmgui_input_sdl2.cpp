#define DMGUI_IMPLEMENTATION
#include "dmgui/dmgui.h"
//#include "dmgui_input_sdl2.h"
#include <SDL_events.h>

int dmguiCreateInputContext(const DmguiParamCreateInputContext* in, DmguiInputContext* out) {
    out->pumpEvents = in->pumpEvents;
    out->consumeInputEvents = in->consumeInputEvents;
    return 1;
}

int dmguiDestroyInputContext(DmguiInputContext* in) {
    return 1;
}

int dmguiUpdateInputContextBegin(DmguiInputContext* ctx) {

    ctx->posLast = ctx->pos;
    ctx->buttonPressMask = 0;
    ctx->buttonReleaseMask = 0;

    if (ctx->pumpEvents)
    {
        // if using sdl video subsystem, must be called on same thread as video init
        SDL_PumpEvents();
    }

    // todo: app events such as window management, clipboard, dragdrop files etc

    SDL_eventaction action = ctx->consumeInputEvents ? SDL_GETEVENT : SDL_PEEKEVENT;
    SDL_EventType minType = SDL_KEYDOWN;      //SDL_FIRSTEVENT;
    SDL_EventType maxType = SDL_MULTIGESTURE; //SDL_LASTEVENT;

    SDL_Event events[DMGUI_EVENT_MAX_COUNT];
    int eventCount = SDL_PeepEvents(events, DMGUI_EVENT_MAX_COUNT,
        action, minType, maxType);

    const SDL_Event* event = events;
    const SDL_Event* eventEnd = events + eventCount;

    // if mouse pos in motion events are not good enough, then poll mouse directly
    // this would override the input position, so should only be used in mouse mode
    //int mouseX, mouseY;
    //ctx->HoldMask = SDL_GetMouseState(&mouseX, &mouseY);
    //ctx->Pos = { (float)mouseX, (float)mouseY };

    ////
    // these belong on context obj
    static float sensitivity = 0.1f; // todo: user set sensitivity, then should be scaled by canvas/viewport
    static float velocityX = 0.f;
    static float velocityY = 0.f;
    //

    for (; event != eventEnd; ++event)
    {
        switch (event->type)
        {
        ////
        // mouse
        case SDL_MOUSEMOTION:
            ctx->pos = { (float)event->motion.x, (float)event->motion.y };
            ctx->buttonHoldMask = event->motion.state;
            break;
        case SDL_MOUSEWHEEL:
            ctx->scroll = { event->wheel.preciseX, event->wheel.preciseY };
            break;
        // buttons are 0 based index, SDL_BUTTON_LEFT == 0
        case SDL_MOUSEBUTTONDOWN:
            ctx->buttonPressMask |= 1 << (event->button.button - 1);
            break;
        case SDL_MOUSEBUTTONUP:
            ctx->buttonReleaseMask |= 1 << (event->button.button - 1);
            break;
        ////
        // joystick
        case SDL_JOYAXISMOTION:
            velocityX += event->jaxis.axis == 0 ? (float)event->jaxis.value * sensitivity : 0.f;
            velocityY += event->jaxis.axis == 1 ? (float)event->jaxis.value * sensitivity : 0.f;
            ctx->pos.x += velocityX;
            ctx->pos.y += velocityY;
            break;
        case SDL_JOYBUTTONDOWN:
            ctx->buttonPressMask |= 1 << (event->jbutton.button - 1);
            ctx->buttonHoldMask |= 1 << (event->jbutton.button - 1);
            break;
        case SDL_JOYBUTTONUP:
            ctx->buttonReleaseMask |= 1 << (event->jbutton.button - 1);
            ctx->buttonHoldMask &= ~(1 << (event->jbutton.button - 1));
            break;
        ////
        // controller
        case SDL_CONTROLLERAXISMOTION:
            velocityX = event->caxis.axis == SDL_CONTROLLER_AXIS_LEFTX ? (float)event->caxis.value * sensitivity : 0.f;
            velocityY = event->caxis.axis == SDL_CONTROLLER_AXIS_LEFTY ? (float)event->caxis.value * sensitivity : 0.f;
            ctx->pos.x += velocityX;
            ctx->pos.y += velocityY;
            break;
        case SDL_CONTROLLERBUTTONDOWN:
            ctx->buttonPressMask |= 1 << (event->cbutton.button - 1);
            ctx->buttonHoldMask |= 1 << (event->cbutton.button - 1);
            break;
        case SDL_CONTROLLERBUTTONUP:
            ctx->buttonReleaseMask |= 1 << (event->cbutton.button - 1);
            ctx->buttonHoldMask &= ~(1 << (event->cbutton.button - 1));
            break;
        ////
        // touch
        case SDL_FINGERDOWN:
            ctx->buttonHoldMask |= 1;
            break;
        case SDL_FINGERUP:
            ctx->buttonHoldMask &= ~1;
            break;
        case SDL_FINGERMOTION:
            // tfinger x and y are 0 to 1 pct of viewport - need a way to access viewport data here or hold them separately
            ctx->pos = { (float)event->tfinger.x * 1920, (float)event->tfinger.y * 1080};
            break;
        default:
            break;
        }
    }

    return 1;
}

int dmguiUpdateInputContextEnd(DmguiInputContext* /*ctx*/) {
    return 1;
}
