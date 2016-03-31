//
//  ProgramBuilder.h
//  SFSL
//
//  Created by Romain Beguet on 18.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_ProgramBuilder__
#define __SFSL__API_ProgramBuilder__

#include <iostream>
#include "SetVisibilities.h"
#include "Module.h"
#include "ClassBuilder.h"
#include "TypeConstructorBuilder.h"

DECL_PRIVATE_IMPL_FOR(ProgramBuilder)

namespace sfsl {

class SFSL_API_PUBLIC ProgramBuilder final {
public:

    ~ProgramBuilder();
    operator bool() const;

    Module openModule(const std::string& moduleName) const;

    Type parseType(const std::string& str);
    Type createFunctionType(const std::vector<Type>& argTypes, Type retType);

    ClassBuilder classBuilder(const std::string& className);
    TypeConstructorBuilder typeConstructorBuilder(const std::string& typeConstructorName);

private:
    friend class Compiler;

    ProgramBuilder(PRIVATE_IMPL_PTR(ProgramBuilder) impl);

    PRIVATE_IMPL_PTR(ProgramBuilder) _impl;
};

}

#endif
