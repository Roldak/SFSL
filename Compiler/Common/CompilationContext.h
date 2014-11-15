//
//  CompilationContext.h
//  SFSL
//
//  Created by Romain Beguet on 15.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__CompilationContext__
#define __SFSL__CompilationContext__

#include <iostream>
#include <memory>

#include "MemoryManager.h"
#include "Reporter.h"

namespace sfsl {

namespace common {

class CompilationContext {
public:

    CompilationContext(const CompilationContext& other) = delete;
    virtual ~CompilationContext();

    AbstractMemoryManager& memoryManager();
    AbstractReporter& reporter();

    static std::shared_ptr<CompilationContext> DefaultCompilationContext();

private:

    CompilationContext(AbstractMemoryManager* manager, AbstractReporter* reporter);

    AbstractMemoryManager* _mngr;
    AbstractReporter* _rprt;

};

}

}

#endif
