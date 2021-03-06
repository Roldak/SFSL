//
//  Compiler.h
//  SFSL
//
//  Created by Romain Beguet on 18.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_Compiler__
#define __SFSL__API_Compiler__

#include <vector>
#include <memory>
#include "SetVisibilities.h"
#include "CompilerConfig.h"
#include "ProgramBuilder.h"
#include "Type.h"
#include "Pipeline.h"
#include "AbstractOutputCollector.h"

DECL_PRIVATE_IMPL_FOR(Compiler)

#define COMPILE_PASS(prog, pipeline, args) \
    extern "C" void __declspec(dllexport) __stdcall compilePass(prog, pipeline, args)

namespace sfsl {

class SFSL_API_PUBLIC Compiler final {
public:
    Compiler(const CompilerConfig& config);
    ~Compiler();

    void loadPlugin(const std::string& pathToPluginDll, const std::vector<std::string>& args = {});
    void unloadPlugin(const std::string& pathToPluginDll);

    ProgramBuilder parse(const std::string& srcName, const std::string& srcContent);

    void compile(   ProgramBuilder progBuilder,
                    AbstractOutputCollector& collector,
                    const Pipeline& ppl = Pipeline::createDefault());

private:
    PRIVATE_IMPL_PTR(Compiler) _impl;
};

}

#endif
