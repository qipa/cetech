
#include <cetech/celib/allocator.h>

#include <cetech/modules/input.h>
#include <cetech/kernel/api_system.h>
#include "cetech/modules/luasys.h"
#include "../luasys_private.h"

#define API_NAME "Keyboard"

CETECH_DECL_API(ct_keyboard_a0);

static int _keyboard_button_index(lua_State *l) {
    const char *name = luasys_to_string(l, 1);

    uint32_t idx = ct_keyboard_a0.button_index(name);
    luasys_push_float(l, idx);

    return 1;
}

static int _keyboard_button_name(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_float(l, 1));

    luasys_push_string(l, ct_keyboard_a0.button_name(idx));

    return 1;

}

static int _keyboard_button_state(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_float(l, 1));

    luasys_push_bool(l, ct_keyboard_a0.button_state(0, idx));

    return 1;
}

static int _keyboard_button_pressed(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_float(l, 1));

    luasys_push_bool(l, ct_keyboard_a0.button_pressed(0, idx));

    return 1;

}

static int _keyboard_button_released(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_float(l, 1));

    luasys_push_bool(l, ct_keyboard_a0.button_released(0, idx));

    return 1;

}


void _register_lua_keyboard_api(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_keyboard_a0);

    luasys_add_module_function(API_NAME, "button_index",
                               _keyboard_button_index);
    luasys_add_module_function(API_NAME, "button_name", _keyboard_button_name);
    luasys_add_module_function(API_NAME, "button_state",
                               _keyboard_button_state);
    luasys_add_module_function(API_NAME, "button_pressed",
                               _keyboard_button_pressed);
    luasys_add_module_function(API_NAME, "button_released",
                               _keyboard_button_released);
}