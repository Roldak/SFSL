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
     * @brief The AbstractMemoryManager class
     *
     */
    class AbstractMemoryManager {
    public:

        virtual ~AbstractMemoryManager() = 0;

        template<typename T, typename... Args>
        T* New(Args... args) {
            T* ptr = new T(args...);
            push(ptr);
            return ptr;
        }

    protected:

        virtual void push(MemoryManageable* ptr) = 0;
        virtual void free() = 0;

    };

    template< template<typename> class Collection_t = std::vector >
    class MemoryManager : public AbstractMemoryManager {
    public:

        MemoryManager() {}
        virtual ~MemoryManager() { free(); }

    private:

        virtual void push(MemoryManageable *ptr) {
            _allocated.push_back(ptr);
        }

        virtual void free() {
            for (auto ptr : _allocated) {
                delete ptr;
            }
        }

        Collection_t<MemoryManageable*> _allocated;

    };

}

}

#endif
