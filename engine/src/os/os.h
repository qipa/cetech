#pragma once

#include "platform/defines.h"

#include "common/container/container_types.h"
#include "common/math/math_types.h"
#include "os/os_types.h"

namespace cetech {
    namespace os {
        CE_INLINE void init();
        CE_INLINE void shutdown();

        CE_INLINE void frame_start();
        CE_INLINE void frame_end();

        CE_INLINE uint32_t get_ticks();
    }

    namespace os {
        namespace window {
            enum WindowFlags {
                WINDOW_NOFLAG = 0,
                WINDOW_FULLSCREEN = 1,
                
            };

            enum WindowPos {
                WINDOWPOS_CENTERED = -1,
                WINDOWPOS_UNDEFINED = -2
            };
        }

        namespace window {
            CE_INLINE Window make_window(const char* title,
                                         const int32_t x,
                                         const int32_t y,
                                         const int32_t width,
                                         const int32_t height,
                                         WindowFlags flags);

            CE_INLINE void destroy_window(const Window& w);

            CE_INLINE void set_title(const Window& w, const char* title);
            CE_INLINE const char* get_title(const Window& w);
        }

        namespace keyboard {
            CE_INLINE uint32_t button_index(const char* scancode);
            CE_INLINE const char* button_name(const uint32_t button_index);

            CE_INLINE bool button_state(const uint32_t button_index);
            CE_INLINE bool button_pressed(const uint32_t button_index);
            CE_INLINE bool button_released(const uint32_t button_index);
        };

        namespace mouse {
            CE_INLINE uint32_t button_index(const char* scancode);
            CE_INLINE const char* button_name(const uint32_t button_index);

            CE_INLINE bool button_state(const uint32_t button_index);
            CE_INLINE bool button_pressed(const uint32_t button_index);
            CE_INLINE bool button_released(const uint32_t button_index);

            CE_INLINE Vector2 axis();
        };

        namespace file {
            enum SeekWhence {
                SW_SEEK_SET = 1,
                SW_SEEK_CUR,
                SW_SEEK_END
            };

            CE_INLINE File from_file(const char* path, const char* mode);

            CE_INLINE bool is_null(const File& f);

            CE_INLINE int close(const File& f);
            CE_INLINE size_t read(const File& f, void* ptr, size_t size, size_t maxnum);
            CE_INLINE size_t write(const File& f, const void* ptr, size_t size, size_t num);

            CE_INLINE int64_t seek(const File& f, int64_t offset, SeekWhence whence);
            CE_INLINE int64_t tell(const File& f);

            CE_INLINE size_t size(const File& f);
        }

        namespace dir {
            CE_INLINE bool mkdir(const char* path);
            CE_INLINE bool mkpath(const char* path);

            CE_INLINE void listdir(const char* name, Array < char* >& files);
            CE_INLINE void listdir_free(Array < char* >& files);
        }

        namespace cpu {
            /*!
             * Get cpu core count.
             * \return Core count.
             */
            CE_INLINE uint32_t core_count();
        }

        namespace thread {
            /*!
             * Create thread.
             * \param fce Thread fce.
             * \param name Thread name.
             * \param data Thread data.
             */
            CE_INLINE Thread create_thread(thread_fce_t fce, const char* name, void* data);

            /*!
             * Kill thread.
             * \param thread Thread.
             */
            CE_INLINE void kill(Thread thread);

            /*!
             * Get id for thread.
             * \param thread Thread.
             * \return thread id.
             */
            CE_INLINE uint32_t get_id(Thread thread);

            /*!
             * Get actual thread id.
             * \return thread id.
             */
            CE_INLINE uint32_t id();

            /*!
             * Spin lock.
             * \param lock Spin.
             */
            CE_INLINE void spin_lock(Spinlock& lock);

            /*!
             * Spin unlock.
             * \param lock Spin.
             */
            CE_INLINE void spin_unlock(Spinlock& lock);
        }
    }
}

#ifdef CETECH_RUNTIME_SDL2
    #include "os/sdl2/os.h"
#endif
