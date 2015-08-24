//
//  ManageableUserData.h
//  SFSL
//
//  Created by Romain Beguet on 22.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ManageableUserData__
#define __SFSL__ManageableUserData__

#include <iostream>
#include "MemoryManageable.h"

namespace sfsl {

namespace common {

    /**
     * @brief Interface that can be extended by classes that allow
     * having manageable user data
     */
    class HasManageableUserdata {
    public:

        HasManageableUserdata();
        virtual ~HasManageableUserdata();

        template<typename T>
        void setUserdata(T* userdata) {
            _userdata = userdata;
        }

        template<typename T>
        T* getUserdata() {
            return static_cast<T*>(_userdata);
        }

    private:

        MemoryManageable* _userdata;
    };

}

}

#endif
