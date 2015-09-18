//
//  Compiler.cpp
//  SFSL
//
//  Created by Romain Beguet on 18.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "api/Compiler.h"
#include "Compiler/Common/CompilationContext.h"
#include "Compiler/Frontend/Parser/Parser.h"
#include "Compiler/Frontend/Lexer/InputSource.h"

BEGIN_PRIVATE_DEF

class NAME_OF_IMPL(Compiler) {
public:
    NAME_OF_IMPL(Compiler)(CompCtx_Ptr ctx) : ctx(ctx) {}
    ~NAME_OF_IMPL(Compiler)() {}

    CompCtx_Ptr ctx;
};

class NAME_OF_IMPL(ProgramBuilder) {
public:
    NAME_OF_IMPL(ProgramBuilder)(ast::Program* prog) : _prog(prog) { }
    ~NAME_OF_IMPL(ProgramBuilder)() { }

    ast::Program* _prog;
};

END_PRIVATE_DEF


namespace sfsl {

Compiler::Compiler(const CompilerConfig& config)
    : _impl(NEW_PRIV_IMPL(Compiler)(common::CompilationContext::DefaultCompilationContext(config.getChunkSize()))) {

}

Compiler::~Compiler() {
    delete _impl;
}

ProgramBuilder Compiler::parse(const std::string& srcName, const std::string& srcContent) {
    src::StringSource source(src::InputSourceName::make(_impl->ctx, srcName), srcContent);
    lex::Lexer lexer(_impl->ctx, source);
    ast::Parser parser(_impl->ctx, lexer);
    ast::Program* program = parser.parse();
    return ProgramBuilder(NEW_PRIV_IMPL(ProgramBuilder)(program));
}

ProgramBuilder::ProgramBuilder(PRIVATE_IMPL_PTR(ProgramBuilder) impl) : _impl(impl) {

}

ProgramBuilder::~ProgramBuilder() {
    delete _impl;
}

}
