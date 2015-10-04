//
//  IOPhaseObject.h
//  SFSL
//
//  Created by Romain Beguet on 04.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_IOPhaseObject__
#define __SFSL__API_IOPhaseObject__

#include <iostream>

// To add as the first line of the class declaration
#define IO_PHASE_OBJECT_HEADER \
    public: \
        virtual size_t getId() const { \
            return priv::getIOPhaseObjectID<decltype(this)>(); \
        } \
    private:



namespace sfsl {

namespace priv {

// http://codereview.stackexchange.com/questions/44936/unique-type-id-in-c

template<typename T>
struct IOPhaseObjectUnique {
    static void id() { }
};

template<typename T>
size_t getIOPhaseObjectID() {
    return reinterpret_cast<size_t>(&IOPhaseObjectUnique<T>::id);
}

}

class IOPhaseObject {
public:
    virtual ~IOPhaseObject();

    virtual size_t getId() const = 0;
};

}

#endif
