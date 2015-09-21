//
//  Compiler.cpp
//  SFSL
//
//  Created by Romain Beguet on 18.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "api/Compiler.h"
#include "api/Errors.h"

#include "ModuleContainer.h"

#include "Compiler/Frontend/Parser/Parser.h"
#include "Compiler/Frontend/AST/Visitors/ASTPrinter.h"
#include "Compiler/Frontend/Analyser/NameAnalysis.h"
#include "Compiler/Frontend/Analyser/KindChecking.h"
#include "Compiler/Frontend/Analyser/TypeChecking.h"
#include "Compiler/Frontend/Symbols/SymbolResolver.h"
#include "Compiler/Backend/UserDataAssignment.h"
#include "Compiler/Backend/BytecodeGenerator.h"

#include "Compiler/Frontend/Symbols/Scope.h"

BEGIN_PRIVATE_DEF

class NAME_OF_IMPL(Compiler) {
public:
    NAME_OF_IMPL(Compiler)(CompCtx_Ptr ctx) : ctx(ctx) {}
    ~NAME_OF_IMPL(Compiler)() {}

    CompCtx_Ptr ctx;
};

class NAME_OF_IMPL(Type) {
public:
    NAME_OF_IMPL(Type)(ast::TypeExpression* type) : _type(type) { }
    ~NAME_OF_IMPL(Type)() { }

    ast::TypeExpression* _type;
};

class NAME_OF_IMPL(Module) : public ModuleContainer {
public:
    NAME_OF_IMPL(Module)(common::AbstractMemoryManager& mngr, const std::string& name) : mngr(mngr), _name(name) { }
    virtual ~NAME_OF_IMPL(Module)() { }

    virtual Module createProxyModule(const std::string& name) const {
        return Module(NEW_PRIV_IMPL(Module)(mngr, name));
    }

    virtual ast::ModuleDecl* buildModule(Module m) {
        return m._impl->closeModule();
    }

    ast::ModuleDecl* closeModule() {
        return mngr.New<ast::ModuleDecl>(mngr.New<ast::Identifier>(_name), closeContainer(mngr), _tdecls, _ddecls);
    }

    void typeDef(const std::string& name, Type type) {
        _tdecls.push_back(mngr.New<ast::TypeDecl>(mngr.New<ast::TypeIdentifier>(name), type._impl->_type));
    }

    common::AbstractMemoryManager& mngr;

    const std::string& _name;
    std::vector<ast::TypeDecl*> _tdecls;
    std::vector<ast::DefineDecl*> _ddecls;
};

class NAME_OF_IMPL(ProgramBuilder) : public ModuleContainer {
public:
    NAME_OF_IMPL(ProgramBuilder)(common::AbstractMemoryManager& mngr, ast::Program* prog) : mngr(mngr), _prog(prog) { }
    virtual ~NAME_OF_IMPL(ProgramBuilder)() { }

    virtual Module createProxyModule(const std::string& name) const {
        return Module(NEW_PRIV_IMPL(Module)(mngr, name));
    }

    virtual ast::ModuleDecl* buildModule(Module m) {
        return m._impl->closeModule();
    }

    ast::Program* createUpdatedProgram() {
        if (openModulesCount()) {
            std::vector<ast::ModuleDecl*> modules(closeContainer(mngr));
            modules.insert(modules.end(), _prog->getModules().begin(), _prog->getModules().end());
            return mngr.New<ast::Program>(modules);
        } else {
            return _prog;
        }
    }

    common::AbstractMemoryManager& mngr;

    ast::Program* _prog;
};

END_PRIVATE_DEF


namespace sfsl {

// COMPILER

Compiler::Compiler(const CompilerConfig& config)
    : _impl(NEW_PRIV_IMPL(Compiler)(common::CompilationContext::DefaultCompilationContext(config.getChunkSize()))) {

}

Compiler::~Compiler() {

}

ProgramBuilder Compiler::parse(const std::string& srcName, const std::string& srcContent) {
    src::StringSource source(src::InputSourceName::make(_impl->ctx, srcName), srcContent);
    lex::Lexer lexer(_impl->ctx, source);
    ast::Parser parser(_impl->ctx, lexer);
    ast::Program* program = parser.parse();

    return ProgramBuilder(NEW_PRIV_IMPL(ProgramBuilder)(_impl->ctx->memoryManager(), _impl->ctx->reporter().getErrorCount() == 0 ? program : nullptr));
}

std::vector<std::string> Compiler::compile(ProgramBuilder progBuilder) {
    if (!progBuilder) {
        return {};
    }

    CompCtx_Ptr ctx = _impl->ctx;
    ast::Program* prog = progBuilder._impl->createUpdatedProgram();

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

Type Compiler::parseType(const std::string& str) {
    src::StringSource source(src::InputSourceName::make(_impl->ctx, "type"), str);
    lex::Lexer lexer(_impl->ctx, source);
    ast::Parser parser(_impl->ctx, lexer);
    ast::TypeExpression* tpe = parser.parseType();

    return Type(NEW_PRIV_IMPL(Type)(_impl->ctx->reporter().getErrorCount() == 0 ? tpe : nullptr));
}

// PROGRAM BUILDER

ProgramBuilder::ProgramBuilder(PRIVATE_IMPL_PTR(ProgramBuilder) impl) : _impl(impl) {

}

ProgramBuilder::~ProgramBuilder() {

}

ProgramBuilder::operator bool() const {
    return _impl != nullptr;
}

// MODULE

Module ProgramBuilder::openModule(const std::string& moduleName) const {
    if (_impl) {
        return _impl->openModule(moduleName);
    }
    return Module(NEW_PRIV_IMPL(Module)(_impl->mngr, nullptr));
}

Module::Module(PRIVATE_IMPL_PTR(Module) impl) : _impl(impl) {

}

Module::~Module() {

}

Module::operator bool() const {
    return _impl != nullptr;
}

Module Module::openModule(const std::string& moduleName) const {
    if (_impl) {
        return _impl->openModule(moduleName);
    }
    return Module(NEW_PRIV_IMPL(Module)(_impl->mngr, nullptr));
}

void Module::typeDef(const std::string& typeName, Type type) {
    _impl->typeDef(typeName, type);
}

// TYPE

Type::Type(PRIVATE_IMPL_PTR(Type) impl) : _impl(impl) {

}

Type::~Type() {

}

Type::operator bool() const {
    return _impl != nullptr;
}

}
