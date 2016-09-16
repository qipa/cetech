#include <string.h>

#include "celib/memory/memory.h"
#include "celib/containers/array.h"
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

time_t os_file_mtime(const char *path) {
    struct stat st;
    stat(path, &st);
    return st.st_mtime;
}

void os_dir_list(const char *path,
                 int recursive,
                 struct array_pchar *files,
                 struct allocator *allocator) {

#if defined(CETECH_LINUX)
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(path))) {
        return;
    }

    if (!(entry = readdir(dir))) {
        closedir(dir);
        return;
    }

    do {
        if (recursive && (entry->d_type == 4)) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char tmp_path[1024] = {0};
            int len = 0;

            if (path[strlen(path) - 1] != '/') {
                len = snprintf(tmp_path, sizeof(tmp_path) - 1, "%s/%s/", path, entry->d_name);
            } else {
                len = snprintf(tmp_path, sizeof(tmp_path) - 1, "%s%s/", path, entry->d_name);
            }

            os_dir_list(tmp_path, 1, files, allocator);
        } else {
            size_t size = strlen(path) + strlen(entry->d_name) + 3;
            char *new_path = CE_ALLOCATE(allocator, char, sizeof(char) * size);

            if (path[strlen(path) - 1] != '/') {
                snprintf(new_path, size - 1, "%s/%s", path, entry->d_name);
            } else {
                snprintf(new_path, size - 1, "%s%s", path, entry->d_name);
            }

            ARRAY_PUSH_BACK(pchar, files, new_path);
        }
    } while ((entry = readdir(dir)));

    closedir(dir);
#endif
}

void os_dir_list_free(ARRAY_T(pchar) *files,
                      struct allocator *allocator) {
    for (int i = 0; i < ARRAY_SIZE(files); ++i) {
        CE_DEALLOCATE(allocator, ARRAY_AT(files, i));
    }
}

int os_dir_make(const char *path) {
    struct stat st;
    const int mode = 0775;

    if (stat(path, &st) != 0) {
        if (mkdir(path, mode) != 0 && errno != EEXIST) {
            return 0;
        }
    } else if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        return 0;
    }

    return 1;
}

int os_dir_make_path(const char *path) {
    char *pp;
    char *sp;
    int status = 1;
    char *copypath = strdup(path);

    pp = copypath;
    while (status == 1 && (sp = strchr(pp, '/')) != 0) {
        if (sp != pp) {
            *sp = '\0';
            status = os_dir_make(copypath);
            *sp = '/';
        }

        pp = sp + 1;
    }

    if (status == 1) {
        status = os_dir_make(path);
    }

    free(copypath);
    return status;
}