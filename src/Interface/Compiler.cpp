//
//  Compiler.cpp
//  SFSL
//
//  Created by Romain Beguet on 18.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "api/Compiler.h"
#include "Compiler/Common/CompilationContext.h"

BEGIN_PRIVATE_DEF

class NAME_OF_IMPL(Compiler) {
public:
    NAME_OF_IMPL(Compiler)(CompCtx_Ptr ctx) : ctx(ctx) {}
    ~NAME_OF_IMPL(Compiler)() {}

    CompCtx_Ptr ctx;
};

END_PRIVATE_DEF

namespace sfsl {

Compiler::Compiler(const CompilerConfig& config)
    : _impl(NEW_PRIV_IMPL(Compiler)(common::CompilationContext::DefaultCompilationContext(config.getChunkSize()))) {

}

Compiler::~Compiler() {

}

}
