//
//  ProgramBuilder.h
//  SFSL
//
//  Created by Romain Beguet on 18.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_ProgramBuilder__
#define __SFSL__API_ProgramBuilder__

#include "set_visibilities.h"

DECL_PRIVATE_IMPL_FOR(ProgramBuilder)

namespace sfsl {

class SFSL_API_PUBLIC ProgramBuilder final {

    ~ProgramBuilder();

private:
    friend class Compiler;

    ProgramBuilder(PRIVATE_IMPL_PTR(ProgramBuilder) impl);

    PRIVATE_IMPL_PTR(ProgramBuilder) _impl;
};

}

#endif
