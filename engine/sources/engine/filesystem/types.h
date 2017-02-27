//! \defgroup Filesystem
//! Filesystem
//! \{
#ifndef CETECH_FILESYSTEM_TYPES_H
#define CETECH_FILESYSTEM_TYPES_H

//==============================================================================
// Includes
//==============================================================================

#include "celib/string/stringid.h"
#include "celib/containers/array.h"
#include "celib/types.h"
#include "celib/filesystem/vio_types.h"
#include "engine/config/cvar.h"


//==============================================================================
// Api
//==============================================================================

//! Filesystem API V0
struct FilesystemApiV0 {
    //! Return root dir
    //! \param root Root
    //! \return Root dir or NULL if root is invalid.
    const char *(*filesystem_get_root_dir)(stringid64_t root);

    //! Open file
    //! \param root Root
    //! \param path File path
    //! \param mode Open mode
    //! \return File or NULL
    struct vio *(*filesystem_open)(stringid64_t root,
                                   const char *path,
                                   enum cel_vio_open_mode mode);

    //! Map path for root
    //! \param root Root
    //! \param base_path Path
    void (*filesystem_map_root_dir)(stringid64_t root,
                                    const char *base_path);

    //! Close file
    //! \param file file
    void (*filesystem_close)(struct vio *file);

    //! List directory
    //! \param root Root
    //! \param path Dir path
    //! \param files Output file array
    //! \param allocator Allocator
    void (*filesystem_listdir)(stringid64_t root,
                               const char *path,
                               const char *filter,
                               string_array_t *files,
                               struct cel_allocator *allocator);

    //! Free list directory array
    //! \param files File array
    //! \param allocator Allocator
    void (*filesystem_listdir_free)(string_array_t *files,
                                    struct cel_allocator *allocator);

    //! Create directory in root
    //! \param root Root
    //! \param path Directory path
    //! \return 1 if ok else 0
    int (*filesystem_create_directory)(stringid64_t root,
                                       const char *path);

    //! Get file modified time
    //! \param root Root
    //! \param path File path
    //! \return Modified time
    i64 (*filesystem_get_file_mtime)(stringid64_t root,
                                     const char *path);

    //! Get full path
    //! \param root Root
    //! \param result Result path
    //! \param maxlen Max result len
    //! \param filename Filename
    //! \return 1 if ok else 0
    int (*filesystem_get_fullpath)(stringid64_t root,
                                   char *result,
                                   u64 maxlen,
                                   const char *filename);
};

#endif //CETECH_FILESYSTEM_TYPES_H
//! |}