//
//  Module.h
//  SFSL
//
//  Created by Romain Beguet on 19.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_Module__
#define __SFSL__API_Module__

#include <memory>
#include "SetVisibilities.h"
#include "Type.h"

DECL_PRIVATE_IMPL_FOR(Module)
DECL_PRIVATE_IMPL_FOR(ProgramBuilder)

namespace sfsl {

class SFSL_API_PUBLIC Module final {
public:

    ~Module();
    operator bool() const;

    Module openModule(const std::string& name) const;

    void typeDef(const std::string& typeName, Type type);

private:
    friend class Compiler;
    friend class ProgramBuilder;
    friend class PRIVATE_IMPL(Module);
    friend class PRIVATE_IMPL(ProgramBuilder);

    Module(PRIVATE_IMPL_PTR(Module) impl);

    PRIVATE_IMPL_PTR(Module) _impl;
};

}

#endif
