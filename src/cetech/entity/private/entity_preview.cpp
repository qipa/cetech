//==============================================================================
// Include
//==============================================================================

#include "celib/allocator.h"
#include "celib/map.inl"
#include "celib/buffer.inl"

#include "cetech/hashlib/hashlib.h"
#include "cetech/os/memory.h"
#include "cetech/api/api_system.h"
#include "cetech/log/log.h"
#include "cetech/os/path.h"
#include "cetech/os/vio.h"
#include "cetech/resource/resource.h"
#include <cetech/module/module.h>
#include <cetech/playground//asset_property.h>
#include <cetech/debugui/debugui.h>
#include <cetech/renderer/texture.h>
#include <cetech/playground//entity_property.h>
#include <cetech/entity/entity.h>
#include <cetech/transform/transform.h>
#include <cfloat>
#include <celib/fpumath.h>
#include <cetech/yaml/ydb.h>
#include <cetech/playground/asset_preview.h>

using namespace celib;
using namespace buffer;

//==============================================================================
// GLobals
//==============================================================================

#define _G EntityPreviewGlobals
static struct _G {
    ct_entity ent;
} _G;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_asset_preview_a0);
CETECH_DECL_API(ct_entity_a0);

static void load(const char* filename, uint64_t type, uint64_t name, struct ct_world world){
    ct_entity ent = ct_entity_a0.spawn(world, name);
    _G.ent = ent;

    if(ct_transform_a0.has(world, ent)) {
        ct_transform t = ct_transform_a0.get(world, ent);
        ct_transform_a0.set_position(t, (float[3]){0.0f});
    }
}

static void unload(const char* filename, uint64_t type, uint64_t name, struct ct_world world){
    ct_entity_a0.destroy(world, &_G.ent, 1);
}

static int _init(ct_api_a0 *api) {
    CEL_UNUSED(api);

    _G = {};


    ct_asset_preview_a0.register_type_preview(
            CT_ID64_0("entity"),
            (ct_asset_preview_fce){
                    .load = load,
                    .unload = unload
            });

    return 1;
}

static void _shutdown() {
    ct_asset_preview_a0.unregister_type_preview(CT_ID64_0("entity"));

    _G = {};
}


CETECH_MODULE_DEF(
        entity_preview,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_asset_preview_a0);
            CETECH_GET_API(api, ct_entity_a0);

        },
        {
            CEL_UNUSED(reload);
            _init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);

            _shutdown();
        }
)