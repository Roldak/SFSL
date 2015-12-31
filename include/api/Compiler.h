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
#include "ClassBuilder.h"
#include "TypeConstructorBuilder.h"
#include "Type.h"
#include "Pipeline.h"
#include "AbstractOutputCollector.h"

DECL_PRIVATE_IMPL_FOR(Compiler)

namespace sfsl {

class SFSL_API_PUBLIC Compiler final {
public:
    Compiler(const CompilerConfig& config);
    ~Compiler();

    ProgramBuilder parse(const std::string& srcName, const std::string& srcContent);

    void compile(   ProgramBuilder progBuilder,
                    AbstractOutputCollector& collector,
                    const Pipeline& ppl = Pipeline::createDefault());

    Type parseType(const std::string& str);
    Type createFunctionType(const std::vector<Type>& argTypes, Type retType);

    ClassBuilder classBuilder(const std::string& className);
    TypeConstructorBuilder typeConstructorBuilder(const std::string& typeConstructorName);

private:
    PRIVATE_IMPL_PTR(Compiler) _impl;
};

}

#endif
