//
//  Module.h
//  SFSL
//
//  Created by Romain Beguet on 19.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_Module__
#define __SFSL__API_Module__

#include "SetVisibilities.h"

DECL_PRIVATE_IMPL_FOR(Module)

namespace sfsl {

class SFSL_API_PUBLIC Module final {
public:

    ~Module();
    operator bool() const;

    Module openModule(const std::string& name) const;

private:
    friend class Compiler;
    friend class ProgramBuilder;

    Module(PRIVATE_IMPL_PTR(Module) impl);

    PRIVATE_IMPL_PTR(Module) _impl;
};

}

#endif
