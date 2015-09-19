//
//  Errors.h
//  SFSL
//
//  Created by Romain Beguet on 19.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "api/Errors.h"
#include "Compiler/Common/CompilationContext.h"

namespace sfsl {

CompileError::CompileError(const std::string& msg) : std::runtime_error(msg) {

}

CompileError::~CompileError() {

}

}
