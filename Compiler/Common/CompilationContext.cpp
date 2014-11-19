//
//  CompilationContext.cpp
//  SFSL
//
//  Created by Romain Beguet on 15.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "CompilationContext.h"

namespace sfsl {

namespace common {

CompilationContext::CompilationContext(AbstractMemoryManager* manager, AbstractReporter* reporter) : _mngr(manager), _rprt(reporter) {

}

CompilationContext::~CompilationContext() {
    delete _mngr;
    delete _rprt;
}

AbstractMemoryManager& CompilationContext::memoryManager() {
    return *_mngr;
}

AbstractReporter& CompilationContext::reporter() {
    return *_rprt;
}

std::shared_ptr<CompilationContext> CompilationContext::DefaultCompilationContext() {
    return std::shared_ptr<CompilationContext>(new CompilationContext(new ChunkedMemoryManager(2048), new StandartErrReporter()));
}


}

}
