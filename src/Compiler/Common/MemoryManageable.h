//
//  MemoryManageable.h
//  SFSL
//
//  Created by Romain Beguet on 12.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__MemoryManageable__
#define __SFSL__MemoryManageable__

#include <stddef.h>

namespace sfsl {

namespace common {

    /**
     * @brief An interface that represents an object
     * that can be managed by a #sfsl::common::MemoryManager
     */
    class MemoryManageable {
    public:

        friend class AbstractMemoryManager;

        /**
         * @brief The destructor
         */
        virtual ~MemoryManageable() {}

    protected:

        // delete operator is protected so that the user cannot delete manually an object that is supposed to be managed automatically
        void operator delete  (void*);
        void operator delete[](void*);

    private:
        // to prevent manual allocation
        void* operator new   (size_t);
        void* operator new[] (size_t);
        void* operator new   (size_t, void*);
    };

}

}

#endif
