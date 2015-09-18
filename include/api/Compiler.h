//
//  Compiler.h
//  SFSL
//
//  Created by Romain Beguet on 18.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_Compiler__
#define __SFSL__API_Compiler__

#include "set_visibilities.h"
#include "CompilerConfig.h"

DECL_PRIVATE_IMPL_FOR(Compiler)

namespace sfsl {

class SFSL_API_PUBLIC Compiler final {
public:
    Compiler(const CompilerConfig& config);
    ~Compiler();

private:
    PRIVATE_IMPL_PTR(Compiler) _impl;
};

}

#endif
