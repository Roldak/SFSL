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
#include "ReporterAdapter.h"

#include "Compiler/Frontend/Parser/Parser.h"
#include "Compiler/Frontend/AST/Visitors/ASTPrinter.h"
#include "Compiler/Frontend/Analyser/NameAnalysis.h"
#include "Compiler/Frontend/Analyser/KindChecking.h"
#include "Compiler/Frontend/Analyser/TypeChecking.h"
#include "Compiler/Frontend/Symbols/SymbolResolver.h"
#include "Compiler/Backend/UserDataAssignment.h"
#include "Compiler/Backend/BytecodeGenerator.h"

#include "Compiler/Frontend/Symbols/Scope.h"

#include "PhaseGraph.h"

#define COMPILER_IMPL_NAME          NAME_OF_IMPL(Compiler)
#define TYPE_IMPL_NAME              NAME_OF_IMPL(Type)
#define MODULE_IMPL_NAME            NAME_OF_IMPL(Module)
#define PROGRAMBUILDER_IMPL_NAME    NAME_OF_IMPL(ProgramBuilder)
#define CLASSBUILDER_IMPL_NAME      NAME_OF_IMPL(ClassBuilder)
#define TCBUILDER_IMPL_NAME         NAME_OF_IMPL(TypeConstructorBuilder)

#define COMPILER_IMPL_PTR           PRIVATE_IMPL_PTR(Compiler)
#define TYPE_IMPL_PTR               PRIVATE_IMPL_PTR(Type)
#define MODULE_IMPL_PTR             PRIVATE_IMPL_PTR(Module)
#define PROGRAMBUILDER_IMPL_PTR     PRIVATE_IMPL_PTR(ProgramBuilder)
#define CLASSBUILDER_IMPL_PTR       PRIVATE_IMPL_PTR(ClassBuilder)
#define TCBUILDER_IMPL_PTR          PRIVATE_IMPL_PTR(TypeConstructorBuilder)

#define NEW_COMPILER_IMPL           NEW_PRIV_IMPL(Compiler)
#define NEW_TYPE_IMPL               NEW_PRIV_IMPL(Type)
#define NEW_MODULE_IMPL             NEW_PRIV_IMPL(Module)
#define NEW_PROGRAMBUILDER_IMPL     NEW_PRIV_IMPL(ProgramBuilder)
#define NEW_CLASSBUILDER_IMPL       NEW_PRIV_IMPL(ClassBuilder)
#define NEW_TCBUILDER_IMPL          NEW_PRIV_IMPL(TypeConstructorBuilder)

BEGIN_PRIVATE_DEF

class COMPILER_IMPL_NAME final {
public:
    COMPILER_IMPL_NAME(CompCtx_Ptr ctx) : ctx(ctx) {}
    ~COMPILER_IMPL_NAME() {}

    CompCtx_Ptr ctx;
};

class TYPE_IMPL_NAME final {
public:
    TYPE_IMPL_NAME(ast::TypeExpression* type) : _type(type) { }
    ~TYPE_IMPL_NAME() { }

    ast::TypeExpression* _type;
};

class MODULE_IMPL_NAME final : public ModuleContainer {
public:
    MODULE_IMPL_NAME(common::AbstractMemoryManager& mngr, const std::string& name) : mngr(mngr), _name(name) { }
    virtual ~MODULE_IMPL_NAME() { }

    virtual Module createProxyModule(const std::string& name) const {
        return Module(NEW_MODULE_IMPL(mngr, name));
    }

    virtual ast::ModuleDecl* buildModule(Module m) {
        return m._impl->closeModule();
    }

    ast::ModuleDecl* closeModule() {
        return mngr.New<ast::ModuleDecl>(mngr.New<ast::Identifier>(_name), closeContainer(mngr), _tdecls, _ddecls);
    }

    void externDef(const std::string& defName, Type defType) {
        ast::Identifier* nameId = mngr.New<ast::Identifier>(defName);
        _ddecls.push_back(mngr.New<ast::DefineDecl>(nameId, defType._impl->_type, nullptr, false, true, false));
    }

    void typeDef(const std::string& name, Type type) {
        _tdecls.push_back(mngr.New<ast::TypeDecl>(mngr.New<ast::TypeIdentifier>(name), type._impl->_type));
    }

    common::AbstractMemoryManager& mngr;

    const std::string _name;
    std::vector<ast::TypeDecl*> _tdecls;
    std::vector<ast::DefineDecl*> _ddecls;
};

class PROGRAMBUILDER_IMPL_NAME final : public ModuleContainer {
public:
    PROGRAMBUILDER_IMPL_NAME(common::AbstractMemoryManager& mngr, ast::Program* prog) : mngr(mngr), _prog(prog) { }
    virtual ~PROGRAMBUILDER_IMPL_NAME() { }

    virtual Module createProxyModule(const std::string& name) const {
        return Module(NEW_MODULE_IMPL(mngr, name));
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

class CLASSBUILDER_IMPL_NAME final {
public:
    CLASSBUILDER_IMPL_NAME(common::AbstractMemoryManager& mngr, const std::string& name)
        : _mngr(mngr), _name(name), _isAbstract(false) { }

    ~CLASSBUILDER_IMPL_NAME() { }

    void setAbstract(bool value) {
        _isAbstract = value;
    }

    void addField(const std::string& fieldName, Type fieldType) {
        ast::Identifier* id = _mngr.New<ast::Identifier>(fieldName);
        ast::TypeExpression* tpe = fieldType._impl->_type;
        _fields.push_back(_mngr.New<ast::TypeSpecifier>(id, tpe));
    }

    void addDef(const std::string& defName, Type defType, bool isRedef, bool isExtern, bool isAbstract) {
        ast::Identifier* id = _mngr.New<ast::Identifier>(defName);
        ast::TypeExpression* tpe = defType._impl->_type;
        _defs.push_back(_mngr.New<ast::DefineDecl>(id, tpe, nullptr, isRedef, isExtern, isAbstract));
    }

    Type build() const {
        ast::ClassDecl* clss = _mngr.New<ast::ClassDecl>(_name, nullptr,
                                         std::vector<ast::TypeDecl*>(),
                                         _fields, _defs, _isAbstract);

        return Type(NEW_TYPE_IMPL(clss));
    }

private:

    common::AbstractMemoryManager& _mngr;

    std::string _name;
    bool _isAbstract;

    std::vector<ast::TypeSpecifier*> _fields;
    std::vector<ast::DefineDecl*> _defs;
};

class TCBUILDER_IMPL_NAME final {
public:
    TCBUILDER_IMPL_NAME(common::AbstractMemoryManager& mngr, const std::string& name)
        : _mngr(mngr), _name(name) { }

    ~TCBUILDER_IMPL_NAME() { }

    void setArgs(const std::vector<Type>& args) {
        _args.clear();
        for (Type arg : args) {
            _args.push_back(arg._impl->_type);
        }
    }

    void setRetExpr(Type ret) {
        _ret = ret._impl->_type;
    }

    Type build() const {
        ast::TypeTuple* tt = _mngr.New<ast::TypeTuple>(_args);
        ast::TypeConstructorCreation* tc = _mngr.New<ast::TypeConstructorCreation>(_name, tt, _ret);

        return Type(NEW_TYPE_IMPL(tc));
    }

private:

    common::AbstractMemoryManager& _mngr;

    std::string _name;

    std::vector<ast::TypeExpression*> _args;
    ast::TypeExpression* _ret;
};

END_PRIVATE_DEF

namespace sfsl {

// COMPILER

Compiler::Compiler(const CompilerConfig& config) {
    CompCtx_Ptr ctx;

    if (config.getReporter() == nullptr) {
        ctx = common::CompilationContext::DefaultCompilationContext(config.getChunkSize());
    } else {
        std::unique_ptr<ReporterAdapter> rep(new ReporterAdapter(config.getReporter()));
        ctx = common::CompilationContext::CustomReporterCompilationContext(config.getChunkSize(), std::move(rep));
    }

    _impl = NEW_COMPILER_IMPL(ctx);

}

Compiler::~Compiler() {

}

ProgramBuilder Compiler::parse(const std::string& srcName, const std::string& srcContent) {
    try {
        src::StringSource source(src::InputSourceName::make(_impl->ctx, srcName), srcContent);
        lex::Lexer lexer(_impl->ctx, source);
        ast::Parser parser(_impl->ctx, lexer);
        ast::Program* program = parser.parse();

        if (_impl->ctx->reporter().getErrorCount() == 0) {
            return ProgramBuilder(NEW_PROGRAMBUILDER_IMPL(_impl->ctx->memoryManager(), program));
        } else {
            return ProgramBuilder(nullptr);
        }
    } catch (const common::CompilationFatalError& err) {
        throw CompileError(err.what());
    }
}

void Compiler::compile(ProgramBuilder progBuilder, AbstractOutputCollector& collector, const Pipeline& ppl) {
    if (!progBuilder) {
        return;
    }

    PhaseContext pctx;
    CompCtx_Ptr ctx = _impl->ctx;
    ast::Program* prog = progBuilder._impl->createUpdatedProgram();

    pctx.output("prog", prog);
    pctx.output("ctx", &ctx);

    try {
        std::set<std::shared_ptr<Phase>> phases(ppl.getPhases());
        std::vector<std::shared_ptr<Phase>> sortedPhases(sortPhases(phases));

        for (std::shared_ptr<Phase> phase : sortedPhases) {
            if (!phase->run(pctx)) {
                break;
            }
        }

        collector.collect(pctx);

    } catch (const PhaseGraphResolutionError& graphErr) {
        throw CompileError(graphErr.what());
    } catch (common::CompilationFatalError err) {
        throw CompileError(err.what());
    }
}

Type Compiler::parseType(const std::string& str) {
    src::StringSource source(src::InputSourceName::make(_impl->ctx, "type"), str);
    lex::Lexer lexer(_impl->ctx, source);
    ast::Parser parser(_impl->ctx, lexer);
    ast::TypeExpression* tpe = parser.parseType();

    return Type(NEW_TYPE_IMPL(_impl->ctx->reporter().getErrorCount() == 0 ? tpe : nullptr));
}

Type Compiler::createFunctionType(const std::vector<Type>& argTypes, Type retType) {
    std::vector<ast::TypeExpression*> argTypeExprs(argTypes.size());
    ast::TypeExpression* retTypeExpr = retType._impl->_type;

    std::transform(argTypes.begin(), argTypes.end(), argTypeExprs.begin(), [](Type t) { return t._impl->_type;});

    return Type(NEW_TYPE_IMPL(_impl->ctx->memoryManager().New<ast::FunctionTypeDecl>(argTypeExprs, retTypeExpr)));
}

ClassBuilder Compiler::classBuilder(const std::string& className) {
    return ClassBuilder(NEW_CLASSBUILDER_IMPL(_impl->ctx->memoryManager(), className));
}

TypeConstructorBuilder Compiler::typeConstructorBuilder(const std::string& typeConstructorName) {
    return TypeConstructorBuilder(NEW_TCBUILDER_IMPL(_impl->ctx->memoryManager(), typeConstructorName));
}

// PROGRAM BUILDER

ProgramBuilder::ProgramBuilder(PROGRAMBUILDER_IMPL_PTR impl) : _impl(impl) {

}

ProgramBuilder::~ProgramBuilder() {

}

ProgramBuilder::operator bool() const {
    return _impl != nullptr;
}

Module ProgramBuilder::openModule(const std::string& moduleName) const {
    if (_impl) {
        return _impl->openModule(moduleName);
    }
    return Module(nullptr);
}

// CLASS BUILDER

ClassBuilder::ClassBuilder(CLASSBUILDER_IMPL_PTR impl) : _impl(impl) {

}

ClassBuilder::~ClassBuilder() {

}

ClassBuilder& ClassBuilder::setAbstract(bool value) {
    _impl->setAbstract(value);
    return *this;
}

ClassBuilder& ClassBuilder::addField(const std::string& fieldName, Type fieldType) {
    _impl->addField(fieldName, fieldType);
    return *this;
}

ClassBuilder& ClassBuilder::addExternDef(const std::string& defName, Type defType, bool isRedef) {
    _impl->addDef(defName, defType, isRedef, true, false);
    return *this;
}

ClassBuilder& ClassBuilder::addAbstractDef(const std::string& defName, Type defType) {
    _impl->addDef(defName, defType, false, false, true);
    return *this;
}

Type ClassBuilder::build() const {
    return _impl->build();
}

// TYPE CONSTRUCTOR BUILDER

TypeConstructorBuilder::TypeConstructorBuilder(TCBUILDER_IMPL_PTR impl) : _impl(impl) {

}

TypeConstructorBuilder::~TypeConstructorBuilder() {

}

TypeConstructorBuilder& TypeConstructorBuilder::setArgs(const std::vector<Type>& args) {
    _impl->setArgs(args);
    return *this;
}

TypeConstructorBuilder& TypeConstructorBuilder::setReturn(Type retExpr) {
    _impl->setRetExpr(retExpr);
    return *this;
}

Type TypeConstructorBuilder::build() const {
    return _impl->build();
}

// MODULE

Module::Module(MODULE_IMPL_PTR impl) : _impl(impl) {

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
    return Module(nullptr);
}

void Module::externDef(const std::string& defName, Type defType) {
    if (_impl) {
        _impl->externDef(defName, defType);
    }
}

void Module::typeDef(const std::string& typeName, Type type) {
    if (_impl) {
        _impl->typeDef(typeName, type);
    }
}

// TYPE

Type::Type(TYPE_IMPL_PTR impl) : _impl(impl) {

}

Type::~Type() {

}

Type::operator bool() const {
    return _impl->_type != nullptr;
}

}
