//
//  CompilationContext.cpp
//  SFSL
//
//  Created by Romain Beguet on 15.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "CompilationContext.h"
#include <memory>

namespace sfsl {

namespace common {

CompilationContext::CompilationContext(std::unique_ptr<AbstractMemoryManager> manager, std::unique_ptr<AbstractReporter> reporter)
    : _mngr(std::move(manager)), _rprt(std::move(reporter)) {

}

CompilationContext::~CompilationContext() {

}

AbstractMemoryManager& CompilationContext::memoryManager() const {
    return *_mngr;
}

AbstractReporter& CompilationContext::reporter() const {
    return *_rprt;
}

std::shared_ptr<CompilationContext> CompilationContext::DefaultCompilationContext(size_t chunksize) {
    return std::shared_ptr<CompilationContext>(
                new CompilationContext(std::move(std::unique_ptr<ChunkedMemoryManager>(new ChunkedMemoryManager(chunksize))),
                                       std::move(std::unique_ptr<StandartErrReporter>(new StandartErrReporter()))));
}

std::shared_ptr<CompilationContext> CompilationContext::CustomReporterCompilationContext(size_t chunksize, std::unique_ptr<AbstractReporter> rep) {
    return std::shared_ptr<CompilationContext>(
                new CompilationContext(std::move(std::unique_ptr<ChunkedMemoryManager>(new ChunkedMemoryManager(chunksize))),
                                       std::move(rep)));
}


}

}
