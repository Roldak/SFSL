//
//  Type.h
//  SFSL
//
//  Created by Romain Beguet on 20.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_Type__
#define __SFSL__API_Type__

#include <stdexcept>
#include "SetVisibilities.h"

DECL_PRIVATE_IMPL_FOR(Module)
DECL_PRIVATE_IMPL_FOR(Type)
DECL_PRIVATE_IMPL_FOR(ClassBuilder)
DECL_PRIVATE_IMPL_FOR(TypeConstructorBuilder)

namespace sfsl {

class SFSL_API_PUBLIC Type final {
public:

    ~Type();
    operator bool() const;

private:
    friend class Compiler;
    friend class PRIVATE_IMPL(Module);
    friend class PRIVATE_IMPL(ClassBuilder);
    friend class PRIVATE_IMPL(TypeConstructorBuilder);

    Type(PRIVATE_IMPL_PTR(Type) impl);

    PRIVATE_IMPL_PTR(Type) _impl;
};

}

#endif
