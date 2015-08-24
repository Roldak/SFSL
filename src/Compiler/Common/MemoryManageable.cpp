//
//  MemoryManageable.cpp
//  SFSL
//
//  Created by Romain Beguet on 12.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "MemoryManageable.h"

namespace sfsl {

namespace common {

void MemoryManageable::operator delete(void* o) {
    return ::operator delete(o);
}

void MemoryManageable::operator delete[](void* t) {
    return ::operator delete[](t);
}

void* MemoryManageable::operator new(size_t size) {
    return ::operator new(size);
}

void* MemoryManageable::operator new[](size_t size) {
    return ::operator new[](size);
}

void* MemoryManageable::operator new(size_t size, void* loc) {
    return loc;
}

}

}
