//
//  ManageableWrapper.h
//  SFSL
//
//  Created by Romain Beguet on 09.04.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ManageableWrapper__
#define __SFSL__ManageableWrapper__

#include <iostream>
#include "MemoryManageable.h"

namespace sfsl {

namespace common {

    template<typename T>
    /**
     * @brief Utility class that adapts an non MemoryManageable object so that
     * it can become managed.
     */
    class ManageableWrapper : public common::MemoryManageable {
    public:

        template<typename... Args>
        /**
         * @param args The arguments to forward to the wrapped object constructor
         */
        ManageableWrapper(Args... args) : _value(std::forward<Args>(args)...) {}

        virtual ~ManageableWrapper() {}

        /**
         * @return A reference towards the underlying object
         */
        T& getRef() {
            return _value;
        }

    private:

        T _value;
    };

}

}

#endif
