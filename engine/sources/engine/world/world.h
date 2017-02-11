#ifndef CETECH_WORLD_SYSTEM_H
#define CETECH_WORLD_SYSTEM_H

//==============================================================================
// Includes
//==============================================================================
#include <celib/string/stringid.h>
#include <engine/entcom/types.h>
#include "types.h"


//==============================================================================
// Interface
//==============================================================================


void world_register_callback(world_callbacks_t clb);

world_t world_create();

void world_destroy(world_t world);

void world_update(world_t world,
                  float dt);

level_t world_load_level(world_t world,
                         stringid64_t name);

entity_t level_unit_by_id(level_t level,
                          stringid64_t name);

entity_t level_unit(level_t level);

#endif //CETECH_WORLD_SYSTEM_H
