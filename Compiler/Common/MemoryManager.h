//
//  MemoryManager.h
//  SFSL
//
//  Created by Romain Beguet on 12.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__MemoryManager__
#define __SFSL__MemoryManager__

#include <iostream>
#include <vector>
#include "MemoryManageable.h"

namespace sfsl {

namespace common {

    /**
     * @brief Represents an abstract Memory Manager which is used throughout the compilation process
     * to manage #sfsl::common::MemoryManageable objects. Objects instantiated through this class
     * should be freed when the MemoryManager is destroyed
     */
    class AbstractMemoryManager {
    public:

        /**
         * @brief ~AbstractMemoryManager
         */
        virtual ~AbstractMemoryManager() = 0;

        template<typename T, typename... Args>
        /**
         * @brief Instantiates and starts managing an object that inherits sfsl::common::MemoryManageable
         * @param args The parameters of class's constructor
         * @return A pointer to the instance
         */
        T* New(Args... args) {
            void* mem_ptr = alloc(sizeof(T));
            T* ptr = new(mem_ptr) T(args...);
            manage(ptr);
            return ptr;
        }

    protected:

        virtual void* alloc(size_t size) = 0;

        /**
         * @brief starts managing the given pointer
         * @param ptr the pointer on the instance that will be managed
         */
        virtual void manage(MemoryManageable* ptr) = 0;

        /**
         * @brief frees every managed instances
         */
        virtual void free() = 0;

    };

    template< template<typename, typename> class Collection_t = std::vector,
              class Allocator = std::allocator<MemoryManageable*> >
    /**
     * @brief The MemoryManager class
     * Represents a concrete MemoryManager object that uses a collection to be specified
     * in order to store pointer to allocated instances
     */
    class DynamicMemoryManager : public AbstractMemoryManager {
    public:

        /**
         * @brief Creates a MemoryManager with the wanted collection type
         */
        DynamicMemoryManager() {}

        /**
         * @brief Destroys the MemoryManager and frees every instances that are stored
         */
        virtual ~DynamicMemoryManager() { free(); }

    private:

        virtual void* alloc(size_t size) {
            return static_cast<void*>(new char[size]);
        }

        virtual void manage(MemoryManageable *ptr) {
            _allocated.push_back(ptr);
        }

        virtual void free() {
            for (auto ptr : _allocated) {
                delete ptr;
            }
        }

        Collection_t<MemoryManageable*, Allocator> _allocated;

    };

    class MemoryChunk {
    public:
        MemoryChunk(size_t size);
        virtual ~MemoryChunk();

        static void* alloc(MemoryChunk*& chunk, size_t size);

    private:

        char* _chunk;
        size_t _chunkSize;
        size_t _offset;

        MemoryChunk* _parent;
    };

    class ChunkedMemoryManager : public AbstractMemoryManager {
    public:

        ChunkedMemoryManager(size_t chunksSize);
        virtual ~ChunkedMemoryManager() { free(); }

    private:

        virtual void* alloc(size_t size) {
            return MemoryChunk::alloc(_lastChunk, size);
        }

        virtual void manage(MemoryManageable *ptr) {
            _allocated.push_back(ptr);
        }

        virtual void free() {
            for (auto ptr : _allocated) {
                ptr->~MemoryManageable();
            }
            delete _lastChunk;
        }

        MemoryChunk* _lastChunk;
        std::vector<MemoryManageable*> _allocated;

    };

}

}

#endif
