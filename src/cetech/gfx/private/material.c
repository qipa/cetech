//==============================================================================
// Include
//==============================================================================
#include <stdio.h>
#include <string.h>

#include "celib/allocator.h"

#include "celib/hashlib.h"
#include "celib/memory.h"
#include "celib/api_system.h"


#include "cetech/resource/resource.h"

#include <cetech/gfx/renderer.h>
#include <cetech/gfx/material.h>
#include <cetech/gfx/shader.h>

#include <cetech/gfx/texture.h>
#include <celib/module.h>

#include <celib/os.h>
#include <celib/macros.h>
#include <cetech/editor/asset_property.h>
#include <cetech/gfx/debugui.h>
#include <cetech/ecs/ecs.h>
#include <cetech/gfx/mesh_renderer.h>
#include <cetech/editor/asset_preview.h>
#include <cetech/editor/editor_ui.h>
#include <celib/log.h>
#include <cetech/resource/builddb.h>

#include "material.h"

int materialcompiler_init(struct ce_api_a0 *api);

//==============================================================================
// Defines
//==============================================================================

#define _G MaterialGlobals
#define LOG_WHERE "material"


//==============================================================================
// GLobals
//==============================================================================

static struct _G {
    struct ce_cdb_t db;
    struct ce_alloc *allocator;
} _G;


//==============================================================================
// Resource
//==============================================================================

static ct_render_uniform_type_t _type_to_bgfx[] = {
        [MAT_VAR_NONE] = CT_RENDER_UNIFORM_TYPE_COUNT,
        [MAT_VAR_INT] = CT_RENDER_UNIFORM_TYPE_INT1,
        [MAT_VAR_TEXTURE] = CT_RENDER_UNIFORM_TYPE_INT1,
        [MAT_VAR_TEXTURE_HANDLER] = CT_RENDER_UNIFORM_TYPE_INT1,
        [MAT_VAR_VEC4] = CT_RENDER_UNIFORM_TYPE_VEC4,
        [MAT_VAR_MAT44] = CT_RENDER_UNIFORM_TYPE_MAT4,
};

static void online(uint64_t name,
                   uint64_t obj) {
    CE_UNUSED(name);
    uint64_t layers_obj = ce_cdb_a0->read_subobject(obj, MATERIAL_LAYERS, 0);

    const uint64_t layers_n = ce_cdb_a0->prop_count(layers_obj);
    uint64_t layers_keys[layers_n];
    ce_cdb_a0->prop_keys(layers_obj, layers_keys);

    for (int i = 0; i < layers_n; ++i) {
        uint64_t layer_obj = ce_cdb_a0->read_subobject(layers_obj,
                                                       layers_keys[i], 0);

        uint64_t variables_obj = ce_cdb_a0->read_subobject(layer_obj,
                                                           MATERIAL_VARIABLES_PROP,
                                                           0);
        const uint64_t variables_n = ce_cdb_a0->prop_count(variables_obj);
        uint64_t variables_keys[variables_n];
        ce_cdb_a0->prop_keys(variables_obj, variables_keys);

        for (int k = 0; k < variables_n; ++k) {
            uint64_t var_obj = ce_cdb_a0->read_subobject(variables_obj,
                                                         variables_keys[k], 0);

            const char *uniform_name = ce_cdb_a0->read_str(var_obj,
                                                           MATERIAL_VAR_NAME_PROP,
                                                           0);
            uint64_t type = ce_cdb_a0->read_uint64(var_obj,
                                                   MATERIAL_VAR_TYPE_PROP, 0);

            const ct_render_uniform_handle_t handler = \
                ct_renderer_a0->create_uniform(uniform_name,
                                               _type_to_bgfx[type], 1);

            ce_cdb_obj_o *var_w = ce_cdb_a0->write_begin(var_obj);
            ce_cdb_a0->set_uint64(var_w, MATERIAL_VAR_HANDLER_PROP,
                                  handler.idx);
            ce_cdb_a0->write_commit(var_w);
        }
    }
}

static void offline(uint64_t name,
                    uint64_t obj) {
    CE_UNUSED(name, obj);
}

static uint64_t cdb_type() {
    return MATERIAL_TYPE;
}

static void ui_vec4(struct ct_resource_id rid,
                    uint64_t var) {
    const char *str = ce_cdb_a0->read_str(var, MATERIAL_VAR_NAME_PROP, "");
    ct_editor_ui_a0->ui_vec4(rid, var, MATERIAL_VAR_VALUE_PROP, str, 0, 0);
}

static void ui_color4(struct ct_resource_id rid,
                      uint64_t var) {
    const char *str = ce_cdb_a0->read_str(var, MATERIAL_VAR_NAME_PROP, "");
    ct_editor_ui_a0->ui_color(rid, var, MATERIAL_VAR_VALUE_PROP, str, 0, 0);
}

static void ui_texture(struct ct_resource_id rid,
                       uint64_t variable) {
    const char *name = ce_cdb_a0->read_str(variable, MATERIAL_VAR_NAME_PROP,
                                           "");

    ct_editor_ui_a0->ui_resource(rid, variable, MATERIAL_VAR_VALUE_PROP, name,
                                 TEXTURE_TYPE, variable);
}

static void draw_property(struct ct_resource_id rid,
                          uint64_t material) {
    uint64_t layers_obj = ce_cdb_a0->read_ref(material, MATERIAL_LAYERS, 0);

    if (!layers_obj) {
        return;
    }

    uint64_t layer_count = ce_cdb_a0->prop_count(layers_obj);
    uint64_t layer_keys[layer_count];
    ce_cdb_a0->prop_keys(layers_obj, layer_keys);

    for (int i = 0; i < layer_count; ++i) {
        uint64_t layer;
        layer = ce_cdb_a0->read_subobject(layers_obj, layer_keys[i], 0);

        const char *layer_name = ce_cdb_a0->read_str(layer,
                                                     MATERIAL_LAYER_NAME, NULL);

        if (ct_debugui_a0->TreeNodeEx("Layer",
                                      DebugUITreeNodeFlags_DefaultOpen)) {
            ct_debugui_a0->NextColumn();
            ct_debugui_a0->Text("%s", layer_name);
            ct_debugui_a0->NextColumn();

            ct_editor_ui_a0->ui_str(rid, layer, MATERIAL_LAYER_NAME,
                                    "Layer name", i);

            ct_editor_ui_a0->ui_resource(rid, layer, MATERIAL_SHADER_PROP,
                                         "Shader",
                                         SHADER_TYPE, i);

            uint64_t variables;
            variables = ce_cdb_a0->read_ref(layer, MATERIAL_VARIABLES_PROP, 0);

            uint64_t count = ce_cdb_a0->prop_count(variables);
            uint64_t keys[count];
            ce_cdb_a0->prop_keys(variables, keys);

            for (int j = 0; j < count; ++j) {
                uint64_t var;
                var = ce_cdb_a0->read_subobject(variables, keys[j], 0);

                if (!var) {
                    continue;
                }

                const char *type = ce_cdb_a0->read_str(var,
                                                       MATERIAL_VAR_TYPE_PROP,
                                                       0);
                if (!type) continue;

                if (!strcmp(type, "texture")) {
                    ui_texture(rid, var);
                } else if (!strcmp(type, "vec4")) {
                    ui_vec4(rid, var);
                } else if (!strcmp(type, "color")) {
                    ui_color4(rid, var);
                } else if (!strcmp(type, "mat4")) {
                }
            }

            ct_debugui_a0->TreePop();

        } else {
            ct_debugui_a0->NextColumn();
            ct_debugui_a0->Text("%s", layer_name);
            ct_debugui_a0->NextColumn();
        }
    }
}


static const char *display_name() {
    return "Material";
}

static struct ct_asset_property_i0 ct_asset_property_i0 = {
        .display_name = display_name,
        .draw = draw_property,
};

static struct ct_entity load(struct ct_resource_id resourceid,
                             struct ct_world world) {

    struct ct_entity ent = ct_ecs_a0->entity->spawn(world,
                                                    ce_id_a0->id64(
                                                            "core/cube"));

    struct ct_mesh *mesh;
    mesh = ct_ecs_a0->component->get_one(world, MESH_RENDERER_COMPONENT, ent);
    mesh->material = ct_material_a0->create(resourceid.name);

    return ent;
}

static void unload(struct ct_resource_id resourceid,
                   struct ct_world world,
                   struct ct_entity entity) {
    ct_ecs_a0->entity->destroy(world, &entity, 1);
}

static struct ct_asset_preview_i0 ct_asset_preview_i0 = {
        .load = load,
        .unload = unload,
};

static void *get_interface(uint64_t name_hash) {
    if (name_hash == ASSET_PROPERTY) {
        return &ct_asset_property_i0;
    }

    if (name_hash == ASSET_PREVIEW) {
        return &ct_asset_preview_i0;
    }
    return NULL;
}

uint64_t material_compiler(const char *filename,
                           uint64_t k,
                           struct ct_resource_id rid,
                           const char *fullname);

static struct ct_resource_i0 ct_resource_i0 = {
        .cdb_type = cdb_type,
        .online = online,
        .offline = offline,
        .compilator = material_compiler,
        .get_interface = get_interface
};


//==============================================================================
// Interface
//==============================================================================

static uint64_t create(uint64_t name) {
    struct ct_resource_id rid = (struct ct_resource_id) {
            .type = MATERIAL_TYPE,
            .name = name,
    };

    uint64_t object = ct_resource_a0->get(rid);
    return object;
}

static void set_texture_handler(uint64_t material,
                                uint64_t layer,
                                const char *slot,
                                struct ct_render_texture_handle texture) {
    uint64_t layers_obj = ce_cdb_a0->read_ref(material, MATERIAL_LAYERS, 0);
    uint64_t layer_obj = ce_cdb_a0->read_ref(layers_obj, layer, 0);
    uint64_t variables = ce_cdb_a0->read_ref(layer_obj,
                                             MATERIAL_VARIABLES_PROP,
                                             0);
    uint64_t var = ce_cdb_a0->read_ref(variables,
                                       ce_id_a0->id64(slot), 0);
    if (!var) {
        uint64_t name = ce_cdb_a0->read_uint64(material, RESOURCE_NAME_PROP, 0);

        char fullname[128] = {};
        ct_builddb_a0->get_fullname(fullname, CE_ARRAY_LEN(fullname),
                                    MATERIAL_TYPE, name);

        ce_log_a0->warning(LOG_WHERE, "invalid slot: %s for %s", slot,
                           fullname);
        return;
    }

    ce_cdb_obj_o *writer = ce_cdb_a0->write_begin(var);
    ce_cdb_a0->set_uint64(writer, MATERIAL_VAR_VALUE_PROP, texture.idx);
    ce_cdb_a0->set_uint64(writer, MATERIAL_VAR_TYPE_PROP,
                          MAT_VAR_TEXTURE_HANDLER);
    ce_cdb_a0->write_commit(writer);
}

static void submit(uint64_t material,
                   uint64_t _layer,
                   uint8_t viewid) {
    uint64_t layers_obj = ce_cdb_a0->read_ref(material, MATERIAL_LAYERS, 0);
    if (!layers_obj) {
        return;
    }

    uint64_t layer = ce_cdb_a0->read_ref(layers_obj, _layer, 0);

    if (!layer) {
        return;
    }

    uint64_t variables = ce_cdb_a0->read_ref(layer,
                                             MATERIAL_VARIABLES_PROP,
                                             0);

    uint64_t key_count = ce_cdb_a0->prop_count(variables);
    uint64_t keys[key_count];
    ce_cdb_a0->prop_keys(variables, keys);

    uint8_t texture_stage = 0;

    for (int j = 0; j < key_count; ++j) {
        uint64_t var = ce_cdb_a0->read_ref(variables, keys[j], 0);
        uint64_t type = ce_cdb_a0->read_uint64(var, MATERIAL_VAR_TYPE_PROP, 0);

        ct_render_uniform_handle_t handle = {
                .idx = (uint16_t) ce_cdb_a0->read_uint64(var,
                                                         MATERIAL_VAR_HANDLER_PROP,
                                                         0)
        };

        switch (type) {
            case MAT_VAR_NONE:
                break;

            case MAT_VAR_INT: {
                uint32_t v = ce_cdb_a0->read_uint64(var,
                                                    MATERIAL_VAR_VALUE_PROP, 0);
                ct_renderer_a0->set_uniform(handle, &v, 1);
            }
                break;

            case MAT_VAR_TEXTURE: {
                uint64_t t = ce_cdb_a0->read_uint64(var,
                                                    MATERIAL_VAR_VALUE_PROP, 0);
                ct_render_texture_handle_t texture = ct_texture_a0->get(t);
                ct_renderer_a0->set_texture(texture_stage++, handle,
                                            texture, 0);
            }
                break;

            case MAT_VAR_TEXTURE_HANDLER: {
                uint64_t t = ce_cdb_a0->read_uint64(var,
                                                    MATERIAL_VAR_VALUE_PROP, 0);
                ct_renderer_a0->set_texture(texture_stage++, handle,
                                            (ct_render_texture_handle_t) {.idx=(uint16_t) t},
                                            0);
            }
                break;

            case MAT_VAR_COLOR4:
            case MAT_VAR_VEC4: {
                float v[4] = {1.0f, 1.0f, 1.0f, 1.0f};
                ce_cdb_a0->read_vec4(var, MATERIAL_VAR_VALUE_PROP, v);
                ct_renderer_a0->set_uniform(handle, &v, 1);
            }
                break;

            case MAT_VAR_MAT44:
                break;
        }
    }

    uint64_t shader_obj = ct_resource_a0->get(
            (struct ct_resource_id) {
                    .name = ce_cdb_a0->read_uint64(layer,
                                                   MATERIAL_SHADER_PROP,
                                                   0),
                    .type = SHADER_TYPE,
            });

    if (!shader_obj) {
        return;
    }

    ct_render_program_handle_t shader = ct_shader_a0->get(shader_obj);

    uint64_t state = ce_cdb_a0->read_uint64(layer, MATERIAL_STATE_PROP, 0);

    ct_renderer_a0->set_state(state, 0);
    ct_renderer_a0->submit(viewid, shader, 0, false);
}

static struct ct_material_a0 material_api = {
        .create = create,
        .set_texture_handler = set_texture_handler,
        .submit = submit
};

struct ct_material_a0 *ct_material_a0 = &material_api;

static int init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .db = ce_cdb_a0->db()
    };
    api->register_api("ct_material_a0", &material_api);

    ce_api_a0->register_api(RESOURCE_I_NAME, &ct_resource_i0);

    materialcompiler_init(api);

    return 1;
}

static void shutdown() {
    ce_cdb_a0->destroy_db(_G.db);
}

CE_MODULE_DEF(
        material,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ct_resource_a0);
            CE_INIT_API(api, ce_os_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ct_texture_a0);
            CE_INIT_API(api, ct_shader_a0);
            CE_INIT_API(api, ce_cdb_a0);
            CE_INIT_API(api, ct_renderer_a0);
        },
        {
            CE_UNUSED(reload);
            init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);

            shutdown();
        }
)