//==============================================================================
// Includes
//==============================================================================

#include <engine/plugin/plugin.h>
#include "celib/machine/machine.h"

#include "engine/input/keyboard.h"

#include "keystr.h"

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "keyboard"


//==============================================================================
// Globals
//==============================================================================


static struct G {
    u8 state[512];
    u8 last_state[512];
} _G = {0};

static void _init(get_api_fce_t get_engine_api) {
    _G = (struct G) {0};

    log_debug(LOG_WHERE, "Init");
}

static void _shutdown() {
    log_debug(LOG_WHERE, "Shutdown");

    _G = (struct G) {0};
}

static void _update() {
    struct event_header *event = machine_event_begin();

    memory_copy(_G.last_state, _G.state, 512);

    u32 size = 0;
    while (event != machine_event_end()) {
        size = size + 1;

        switch (event->type) {
            case EVENT_KEYBOARD_DOWN:
                _G.state[((struct keyboard_event *) event)->keycode] = 1;
                break;

            case EVENT_KEYBOARD_UP:
                _G.state[((struct keyboard_event *) event)->keycode] = 0;
                break;

            default:
                break;
        }

        event = machine_event_next(event);
    }
}


void *keyboard_get_plugin_api(int api,
                              int version) {

    if (api == PLUGIN_EXPORT_API_ID && version == 0) {
        static struct plugin_api_v0 plugin = {0};

        plugin.init = _init;
        plugin.shutdown = _shutdown;
        plugin.update = _update;

        return &plugin;

    } else if (api == KEYBOARD_API_ID && version == 0) {
        static struct KeyboardApiV1 api_v1 = {
                .button_index = keyboard_button_index,
                .button_name = keyboard_button_name,
                .button_state = keyboard_button_state,
                .button_pressed = keyboard_button_pressed,
                .button_released = keyboard_button_released,
        };

        return &api_v1;
    }

    return 0;
}

//==============================================================================
// Interface
//==============================================================================

u32 keyboard_button_index(const char *button_name) {
    for (u32 i = 0; i < KEY_MAX; ++i) {
        if (!_key_to_str[i]) {
            continue;
        }

        if (cel_strcmp(_key_to_str[i], button_name)) {
            continue;
        }

        return i;
    }

    return 0;
}

const char *keyboard_button_name(const u32 button_index) {
    CEL_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < KEY_MAX));

    return _key_to_str[button_index];
}

int keyboard_button_state(u32 idx,
                          const u32 button_index) {
    CEL_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < KEY_MAX));

    return _G.state[button_index];
}

int keyboard_button_pressed(u32 idx,
                            const u32 button_index) {
    CEL_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < KEY_MAX));

    return _G.state[button_index] && !_G.last_state[button_index];
}

int keyboard_button_released(u32 idx,
                             const u32 button_index) {
    CEL_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < KEY_MAX));

    return !_G.state[button_index] && _G.last_state[button_index];
}
