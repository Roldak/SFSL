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
    struct SFSL_API_PUBLIC Parameter final {
        enum V_TYPE {
            V_IN,
            V_OUT,
            V_NONE
        };

        Parameter(const std::string& name, V_TYPE varType = V_NONE);

        const std::string& getName() const;
        V_TYPE getVarianceType() const;

    private:

        std::string _name;
        V_TYPE _varType;
    };
    ~TypeConstructorBuilder();
    operator bool() const;

    TypeConstructorBuilder& setParams(const std::vector<Parameter>& args);
    TypeConstructorBuilder& setReturn(Type retExpr);

    Type build() const;

private:
    friend class ProgramBuilder;

    TypeConstructorBuilder(PRIVATE_IMPL_PTR(TypeConstructorBuilder) impl);

    PRIVATE_IMPL_PTR(TypeConstructorBuilder) _impl;
};

}

#endif
