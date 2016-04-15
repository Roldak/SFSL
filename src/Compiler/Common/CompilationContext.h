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
#include <map>
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
class CompilationContext final {
public:

    CompilationContext(const CompilationContext& other) = delete;
    ~CompilationContext();

    /**
     * @return The memory manager
     */
    AbstractMemoryManager& memoryManager() const;

    /**
     * @return The info/warning/error reporter
     */
    AbstractReporter& reporter() const;

    template<typename T>
    /**
     * @brief Used to retrieve user data element of a given them from this compilation context.
     * If no entry of the given name were found, the entry is automatically created,
     * using the default constructor of the required type, which is therefore required
     *
     * @param name The name of the user data to retrieve (or insert)
     * @return The user data associated to the given name
     */
    T* retrieveContextUserData(const std::string& name);

    /**
     * @return Creates the default CompilationContext, which uses :
     *  - ChunkedMemoryManager as the memory manager.
     *  - StandartErrReporter as the error reporter.
     */
    static std::shared_ptr<CompilationContext> DefaultCompilationContext(size_t chunksize);

    /**
     * @return Creates a CompilationContext the default memory manager
     * and a custom reporter.
     */
    static std::shared_ptr<CompilationContext> CustomReporterCompilationContext(size_t chunksize, std::unique_ptr<AbstractReporter> rep);

private:

    CompilationContext(std::unique_ptr<AbstractMemoryManager> manager, std::unique_ptr<AbstractReporter> reporter);

    std::unique_ptr<AbstractMemoryManager> _mngr;
    std::unique_ptr<AbstractReporter> _rprt;

    std::map<std::string, MemoryManageable*> _ctxUserData;
};

template<typename T>
T* CompilationContext::retrieveContextUserData(const std::string& name) {
    auto it = _ctxUserData.insert(std::make_pair(name, nullptr));
    if (it.second) {
        it.first->second = _mngr->New<T>();
    }
    return static_cast<T*>(it.first->second);
}

}

typedef std::shared_ptr<common::CompilationContext> CompCtx_Ptr;

}

#endif
