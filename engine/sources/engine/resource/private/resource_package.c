//==============================================================================
// Includes
//==============================================================================

#include <celib/string/stringid.h>
#include <celib/task/task.h>
#include <engine/resource/resource.h>
#include <celib/memory/memory.h>
#include <celib/os/vio.h>
#include <celib/yaml/yaml.h>
#include <celib/log/log.h>
#include <celib/containers/array.h>
#include <celib/memory/memory.h>
#include <celib/os/thread.h>

#include "engine/resource/resource.h"
#include "resource_package.h"

//==============================================================================
// Public interface
//==============================================================================

struct package_task_data {
    stringid64_t name;
};

#define _G PackageGlobals
struct G {
    stringid64_t package_typel;
} _G = {0};

//==============================================================================
// Resource compiler
//==============================================================================

ARRAY_PROTOTYPE(stringid64_t);

struct package_compile_data {
    ARRAY_T(stringid64_t) types;
    ARRAY_T(stringid64_t) name;
    ARRAY_T(u32) name_count;
    ARRAY_T(u32) offset;
};

void forach_clb(yaml_node_t key,
                yaml_node_t value,
                void *data) {
    struct package_compile_data *compile_data = data;

    char type_str[128] = {0};
    char name_str[128] = {0};

    yaml_as_string(key, type_str, CE_ARRAY_LEN(type_str));

    ARRAY_PUSH_BACK(stringid64_t, &compile_data->types, stringid64_from_string(type_str));
    ARRAY_PUSH_BACK(u32, &compile_data->offset, ARRAY_SIZE(&compile_data->name));

    const size_t name_count = yaml_node_size(value);
    ARRAY_PUSH_BACK(u32, &compile_data->name_count, name_count);

    for (int i = 0; i < name_count; ++i) {
        yaml_node_t name_node = yaml_get_seq_node(value, i);
        yaml_as_string(name_node, name_str, CE_ARRAY_LEN(name_str));

        ARRAY_PUSH_BACK(stringid64_t, &compile_data->name, stringid64_from_string(name_str));

        yaml_node_free(name_node);
    }
}

int _package_compiler(const char *filename,
                      struct vio *source_vio,
                      struct vio *build_vio,
                      struct compilator_api *compilator_api) {

    char source_data[vio_size(source_vio) + 1];
    memory_set(source_data, 0, vio_size(source_vio) + 1);
    vio_read(source_vio, source_data, sizeof(char), vio_size(source_vio));

    yaml_document_t h;
    yaml_node_t root = yaml_load_str(source_data, &h);

    struct package_compile_data compile_data = {0};
    ARRAY_INIT(stringid64_t, &compile_data.types, memsys_main_allocator());
    ARRAY_INIT(stringid64_t, &compile_data.name, memsys_main_allocator());
    ARRAY_INIT(u32, &compile_data.offset, memsys_main_allocator());
    ARRAY_INIT(u32, &compile_data.name_count, memsys_main_allocator());

    yaml_node_foreach_dict(root, forach_clb, &compile_data);

    struct package_resource resource = {0};
    resource.type_count = ARRAY_SIZE(&compile_data.types);
    resource.type_offset = sizeof(resource);
    resource.name_count_offset = resource.type_offset + (sizeof(stringid64_t) * ARRAY_SIZE(&compile_data.types));
    resource.name_offset = resource.name_count_offset + (sizeof(u32) * ARRAY_SIZE(&compile_data.name_count));
    resource.offset_offset = resource.name_offset + (sizeof(stringid64_t) * ARRAY_SIZE(&compile_data.name));

    vio_write(build_vio, &resource, sizeof(resource), 1);
    vio_write(build_vio, ARRAY_BEGIN(&compile_data.types), sizeof(stringid64_t), ARRAY_SIZE(&compile_data.types));
    vio_write(build_vio, ARRAY_BEGIN(&compile_data.name_count), sizeof(u32), ARRAY_SIZE(&compile_data.name_count));
    vio_write(build_vio, ARRAY_BEGIN(&compile_data.name), sizeof(stringid64_t), ARRAY_SIZE(&compile_data.name));
    vio_write(build_vio, ARRAY_BEGIN(&compile_data.offset), sizeof(u32), ARRAY_SIZE(&compile_data.offset));

    ARRAY_DESTROY(stringid64_t, &compile_data.types);
    ARRAY_DESTROY(stringid64_t, &compile_data.name);
    ARRAY_DESTROY(u32, &compile_data.offset);
    ARRAY_DESTROY(u32, &compile_data.name_count);

    return 1;
}

int package_init() {
    _G = (struct G) {0};

    _G.package_typel = stringid64_from_string("package");

    resource_compiler_register(_G.package_typel, _package_compiler);

    return 1;
}

void package_shutdown() {

}


void package_task(void *data) {
    struct package_task_data *task_data = data;
    struct package_resource *package = resource_get(_G.package_typel, task_data->name);

    const u32 task_count = package->type_count;
    for (int j = 0; j < task_count; ++j) {
        resource_load_now(package_type(package)[j], &package_name(package)[package_offset(package)[j]],
                          package_name_count(package)[j]);
    }
}

void package_load(stringid64_t name) {

    struct package_task_data *task_data =  CE_ALLOCATE(memsys_main_allocator(), struct package_task_data, 1);

    task_data->name = name;

    taskmanager_add(
            "package_task",
            package_task,
            task_data,
            TASK_AFFINITY_NONE
    );
}

void package_unload(stringid64_t name) {
    struct package_resource *package = resource_get(_G.package_typel, name);

    const u32 task_count = package->type_count;
    for (int j = 0; j < task_count; ++j) {
        resource_unload(package_type(package)[j], &package_name(package)[package_offset(package)[j]], package_name_count(package)[j]);
    }
}

int package_is_loaded(stringid64_t name) {
    const stringid64_t package_type = stringid64_from_string("package");

    struct package_resource *package = resource_get(package_type, name);

    const u32 task_count = package->type_count;
    for (int i = 0; i < task_count; ++i) {
        if (!resource_can_get_all(package_type(package)[i], &package_name(package)[package_offset(package)[i]], package_name_count(package)[i])) {
            return 0;
        }
    }

    return 1;
}

void package_flush(stringid64_t name) {
    while (!package_is_loaded(name)) {
        if(!taskmanager_do_work()) {
            os_thread_yield();
        }
    }
}