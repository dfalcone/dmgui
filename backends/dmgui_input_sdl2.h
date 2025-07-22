#pragma once
#include <stdbool.h>

typedef struct DmguiParamCreateInputContext {
    // whether events should be pumped by gui
    // this is required if you do not pump the events yourself
    bool pumpEvents;
    // whether input events should be consumed by the gui
    // you must call UpdateGuiContextBegin before consuming input events yourself
    // you must make sure any remaining events are removed from the queue afterwards
    bool consumeInputEvents;
} DmguiParamCreateInputContext;

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
#include "dmgui/dmgui.h"

// this is the max number of events on the stack for processing
// the number of events per frame cannot exceed this ammount
#define DMGUI_EVENT_MAX_COUNT 64 // most 2d games and applications
//#define GUI_EVENT_MAX_COUNT 128 // twitch input games
//#define GUI_EVENT_MAX_COUNT 256 // intense high fidelity input

struct DmguiInputContext
{
    dmgui_floatv2_t pos;
    dmgui_floatv2_t posLast;
    dmgui_floatv2_t scroll;

    uint8_t pumpEvents : 1;
    uint8_t consumeInputEvents : 1;

    // whether button is held down (while down)
    uint8_t buttonHoldMask;
    // whether button just pressed (on down)
    uint8_t buttonPressMask;
    // whether button just released (on up)
    uint8_t buttonReleaseMask;
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

// required private internal - called by CreateGuiContext
int dmguiCreateInputContext(const DmguiParamCreateInputContext* in, DmguiInputContext* out);
// required private internal - called by DestroyGuiContext
int dmguiDestroyInputContext(DmguiInputContext* in);
// required private internal - called by UpdateGuiContextBegin
int dmguiUpdateInputContextBegin(DmguiInputContext* ctx);
// required private internal - called by UpdateGuiContextEnd
int dmguiUpdateInputContextEnd(DmguiInputContext* ctx);

#endif