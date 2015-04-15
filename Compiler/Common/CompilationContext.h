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

/**
 * @brief Contains the set of parameters given by the user that have to be used
 * during the compilation to :
 *  - allocate memory
 *  - report errors
 *
 * Instances of this class can be created via the static methods
 */
class CompilationContext {
public:

    CompilationContext(const CompilationContext& other) = delete;
    virtual ~CompilationContext();

    /**
     * @return The memory manager
     */
    AbstractMemoryManager& memoryManager();

    /**
     * @return The info/warning/error reporter
     */
    AbstractReporter& reporter();

    /**
     * @return Creates the default CompilationContext, which uses :
     *  - ChunkedMemoryManager as the memory manager.
     *  - StandartErrReporter as the error reporter.
     */
    static std::shared_ptr<CompilationContext> DefaultCompilationContext();

private:

    CompilationContext(AbstractMemoryManager* manager, AbstractReporter* reporter);

    AbstractMemoryManager* _mngr;
    AbstractReporter* _rprt;

};

}

typedef std::shared_ptr<common::CompilationContext> CompCtx_Ptr;

}

#endif
