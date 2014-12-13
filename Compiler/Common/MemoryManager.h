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
     * @brief Abstract class Representing a Memory Manager which is used throughout the compilation process
     * to manage #sfsl::common::MemoryManageable objects. Objects instantiated through this class
     * will be freed when the MemoryManager is destroyed (if well implemented)
     */
    class AbstractMemoryManager {
    public:

        /**
         * @brief Destructor. Implementations should free all the memory that has been allocated
         */
        virtual ~AbstractMemoryManager() = 0;

        template<typename T, typename... Args>
        /**
         * @brief Instantiates and starts managing an object that inherits sfsl::common::MemoryManageable
         * @param args The parameters of class's constructor
         * @return A pointer to the instance
         */
        T* New(Args... args) {
            return new(alloc(sizeof(T))) T(args...);
        }

    protected:

        /**
         * @brief Allocates a space of size given in parameter by the desired way
         * @param size The size to allocate
         * @return A pointer to the free space
         */
        virtual MemoryManageable* alloc(size_t size) = 0;

    };

    template< template<typename, typename> class Collection_t = std::vector,
              class Allocator = std::allocator<MemoryManageable*> >
    /**
     * @brief Represents a concrete MemoryManager object that uses a collection to be specified
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

        virtual MemoryManageable* alloc(size_t size) {
            MemoryManageable* ptr = reinterpret_cast<MemoryManageable*>(new char[size]);
            _allocated.push_back(ptr);
            return ptr;
        }

        void free() {
            for (auto ptr : _allocated) {
                delete ptr;
            }
        }

        Collection_t<MemoryManageable*, Allocator> _allocated;
    };

    /**
     * @brief Represents a block of memory. It contains a pointer to its parent
     * so that they are destroyed in chain when the last one is destroyed.
     */
    class MemoryChunk {
    public:
        /**
         * @brief Creates a MemoryChunk
         * @param size The size that will have each block of memory
         * @param parent The parent of this chunk
         */
        MemoryChunk(size_t size, MemoryChunk* parent);

        virtual ~MemoryChunk();

        /**
         * @brief Allocates the desired memory in the chunk given in parameter if it contains enough space,
         * otherwise creates an new chunk (and modifies the reference given so that it refers to the new one)
         * and allocates space in this one.
         *
         * @param chunk The MemoryChunk from which to allocate memory
         * @param size The size of the object to be allocated
         * @return A pointer to the allocated space
         */
        static void* alloc(MemoryChunk*& chunk, size_t size);

    private:

        char* _chunk;
        size_t _chunkSize;
        size_t _offset;

        MemoryChunk* _parent;
    };

    /**
     * @brief Represents a concrete MemoryManager object that allocates blocks of memory and
     */
    class ChunkedMemoryManager : public AbstractMemoryManager {
    public:

        /**
         * @brief Creates a ChunkedMemoryManager
         * @param chunksSize The size that will have each chunk of memory
         */
        ChunkedMemoryManager(size_t chunksSize);

        virtual ~ChunkedMemoryManager();

    private:

        virtual MemoryManageable* alloc(size_t size);

        MemoryChunk* _lastChunk;
        std::vector<MemoryManageable*> _allocated;
    };

}

}

#endif
