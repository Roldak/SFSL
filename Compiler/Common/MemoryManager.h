//
//  MemoryManager.h
//  SFSL
//
//  Created by Romain Beguet on 12.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__MemoryManager__
#define __SFSL__MemoryManager__

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
            T* ptr = new T(args...);
            manage(ptr);
            return ptr;
        }

    protected:

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
    class MemoryManager : public AbstractMemoryManager {
    public:

        /**
         * @brief Creates a MemoryManager with the wanted collection type
         */
        MemoryManager() {}

        /**
         * @brief Destroys the MemoryManager and frees every instances that are stored
         */
        virtual ~MemoryManager() { free(); }

    private:

        virtual void manage(MemoryManageable *ptr) {
            _allocated.push_back(ptr);
        }

        virtual void free() {
            for (auto ptr : _allocated) {
                delete ptr;
            }
        }

        std::vector<MemoryManageable*> _allocated;
        //Collection_t<MemoryManageable*, Allocator> _allocated;

    };

}

}

#endif
