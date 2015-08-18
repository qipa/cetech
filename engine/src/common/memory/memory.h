// based on bitsquid foundation
#pragma once

#include <cstring>

#include "common/asserts.h"
#include "common/types.h"

namespace cetech1 {

    /*! Abstract allocator.
     */
    class Allocator {
        public:
            static const uint32_t DEFAULT_ALIGN = 4;              //!< Default align.
            static const uint32_t SIZE_NOT_TRACKED = 0xffffffffu; //!< Size not tracked

        public:
            Allocator() {};
            virtual ~Allocator() {};

        public:
            /*! Allocate memory.
             * \param size Alloc size.
             * \param align align.
             * \return Pointer to new memory.
             */
            virtual void* allocate(const uint32_t size, const uint32_t align = DEFAULT_ALIGN) = 0;

            /*! Deallocate memory.
             * \param p Pointer to memory.
             */
            virtual void deallocate(void* p) = 0;

        public:
            /*! Get allocated size of memory.
             * \param p Pointer to memory.
             * \return Allocated size or SIZE_NOT_TRACKED if allocator does not track size.
             */
            virtual uint32_t allocated_size(void* p) = 0;

            /*! Total allocated memory.
             * \return Total allocated memory or SIZE_NOT_TRACKED if allocator does not track size.
             */
            virtual uint32_t total_allocated() = 0;

        private:
            explicit Allocator(const Allocator& other) {
                (void)other;
            };
            Allocator& operator = (const Allocator &other) {
                (void)other; return *this;
            };
    };

    namespace memory_globals {
        /*! Init global memory.
         */
        void init();

        /*! Shutdown global memory.
         */
        void shutdown();

        /*! Get default allocator.
         * \return Default allocator.
         */
        Allocator& default_allocator();
    }

    namespace memory {
        /*! Allign pointer.
         * \param p Pointer.
         * \param align Align.
         * \return Alligned pointer.
         */
        CE_INLINE void* align_forward(void* p, const uint32_t align);

        CE_INLINE void* pointer_add(void* p, const uint32_t bytes);
        CE_INLINE const void* pointer_add(const void* p, const uint32_t bytes);

        CE_INLINE void* pointer_sub(void* p, const uint32_t bytes);
        CE_INLINE const void* pointer_sub(const void* p, const uint32_t bytes);

        CE_INLINE void* memcpy(void* dst, const void* src, const uint32_t bytes);
        CE_INLINE void* malloc(const uint32_t bytes);
    }

    void* memory::align_forward(void* p, const uint32_t align) {
        uintptr_t pi = uintptr_t(p);
        const uint32_t mod = pi % align;

        if (mod) {
            pi += (align - mod);
        }

        return (void*)pi;
    }

    void* memory::pointer_add(void* p, const uint32_t bytes) {
        CE_CHECK_PTR(p);

        return (void*)((char*)p + bytes);
    }

    const void* memory::pointer_add(const void* p, const uint32_t bytes) {
        CE_CHECK_PTR(p);
        return (const void*)((const char*)p + bytes);
    }

    void* memory::pointer_sub(void* p, const uint32_t bytes) {
        CE_CHECK_PTR(p);
        return (void*)((char*)p - bytes);
    }

    const void* memory::pointer_sub(const void* p, const uint32_t bytes) {
        CE_CHECK_PTR(p);
        return (const void*)((const char*)p - bytes);
    }

    void* memory::memcpy(void* dst, const void* src, const uint32_t bytes) {
        CE_CHECK_PTR(dst);
        return std::memcpy(dst, src, bytes);
    }

    void* memory::malloc(const uint32_t bytes) {
        void* res = std::malloc(bytes);
        CE_CHECK_PTR(res);
        return res;
    }
}