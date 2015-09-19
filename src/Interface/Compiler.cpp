//
//  Compiler.cpp
//  SFSL
//
//  Created by Romain Beguet on 18.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "api/Compiler.h"
#include "api/Errors.h"
#include "Compiler/Frontend/Parser/Parser.h"
#include "Compiler/Frontend/AST/Visitors/ASTPrinter.h"
#include "Compiler/Frontend/Analyser/NameAnalysis.h"
#include "Compiler/Frontend/Analyser/KindChecking.h"
#include "Compiler/Frontend/Analyser/TypeChecking.h"
#include "Compiler/Frontend/Symbols/SymbolResolver.h"
#include "Compiler/Backend/UserDataAssignment.h"
#include "Compiler/Backend/BytecodeGenerator.h"

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
    : _impl(new PRIVATE_IMPL(Compiler)(common::CompilationContext::DefaultCompilationContext(config.getChunkSize()))) {

}

Compiler::~Compiler() {

}

ProgramBuilder Compiler::parse(const std::string& srcName, const std::string& srcContent) {
    src::StringSource source(src::InputSourceName::make(_impl->ctx, srcName), srcContent);
    lex::Lexer lexer(_impl->ctx, source);
    ast::Parser parser(_impl->ctx, lexer);
    ast::Program* program = parser.parse();

    return ProgramBuilder(NEW_PRIV_IMPL(ProgramBuilder)(_impl->ctx->reporter().getErrorCount() == 0 ? program : nullptr));
}

std::vector<std::string> Compiler::compile(ProgramBuilder progBuilder) {
    if (!progBuilder) {
        return {};
    }

    CompCtx_Ptr ctx = _impl->ctx;
    ast::Program* prog = progBuilder._impl->_prog;

    try {

        ast::ScopeGeneration scopeGen(ctx);
        ast::TypeDependencyFixation typeDep(ctx);
        ast::SymbolAssignation symAssign(ctx);

        prog->onVisit(&scopeGen);
        prog->onVisit(&typeDep);
        prog->onVisit(&symAssign);

        if (ctx->reporter().getErrorCount() != 0) {
            return {};
        }

        ast::KindChecking kindCheck(ctx);
        prog->onVisit(&kindCheck);

        if (ctx->reporter().getErrorCount() != 0) {
            return {};
        }

        sym::SymbolResolver res(prog, ctx);
        res.setPredefClassesPath("sfsl.lang");

        ast::TopLevelTypeChecking topleveltypecheck(ctx, res);
        ast::TypeChecking typeCheck(ctx, res);

        prog->onVisit(&topleveltypecheck);
        prog->onVisit(&typeCheck);

        if (ctx->reporter().getErrorCount() != 0) {
            return {};
        }

        out::LinkedListOutput<bc::BCInstruction*> out(ctx);
        bc::UserDataAssignment uda(ctx);
        bc::DefaultBytecodeGenerator gen(ctx, out);

        prog->onVisit(&uda);
        prog->onVisit(&gen);

        if (ctx->reporter().getErrorCount() != 0) {
            return {};
        }

        std::vector<bc::BCInstruction*> instrs(out.toVector());
        std::vector<std::string> instrsStr;

        for (bc::BCInstruction* instr : instrs) {
            instrsStr.push_back(instr->toStringDetailed());
        }

        return instrsStr;

    } catch (common::CompilationFatalError err) {
        throw CompileError(err.what());
    }
}

ProgramBuilder::ProgramBuilder(PRIVATE_IMPL_PTR(ProgramBuilder) impl) : _impl(impl) {

}

ProgramBuilder::~ProgramBuilder() {

}

ProgramBuilder::operator bool() const {
    return _impl != nullptr;
}

}
