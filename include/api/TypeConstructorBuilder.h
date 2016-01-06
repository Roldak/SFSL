//
//  TypeConstructorBuilder.h
//  SFSL
//
//  Created by Romain Beguet on 31.12.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_TypeConstructorBuilder__
#define __SFSL__API_TypeConstructorBuilder__

#include <iostream>
#include <vector>
#include "SetVisibilities.h"

#include "Type.h"

DECL_PRIVATE_IMPL_FOR(TypeConstructorBuilder)

namespace sfsl {

class SFSL_API_PUBLIC TypeConstructorBuilder final {
public:

    ~TypeConstructorBuilder();

    TypeConstructorBuilder& setArgs(const std::vector<Type>& args);
    TypeConstructorBuilder& setReturn(Type retExpr);

    Type build() const;

private:
    friend class Compiler;

    TypeConstructorBuilder(PRIVATE_IMPL_PTR(TypeConstructorBuilder) impl);

    PRIVATE_IMPL_PTR(TypeConstructorBuilder) _impl;
};

}

#endif
