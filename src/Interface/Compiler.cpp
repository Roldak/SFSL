//
//  Compiler.cpp
//  SFSL
//
//  Created by Romain Beguet on 18.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "api/Compiler.h"
#include "api/Errors.h"
#include "api/StandartPrimitiveNamer.h"

#include "ModuleContainer.h"
#include "ReporterAdapter.h"

#include "Compiler/Frontend/Parser/Parser.h"
#include "Compiler/Frontend/AST/Visitors/ASTPrinter.h"
#include "Compiler/Frontend/Analyser/NameAnalysis.h"
#include "Compiler/Frontend/Analyser/KindChecking.h"
#include "Compiler/Frontend/Analyser/TypeChecking.h"
#include "Compiler/Frontend/Symbols/SymbolResolver.h"
#include "Compiler/Backend/AST2BAST/UserDataAssignment.h"
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

#define MAKE_INVALID(T)             (T(PRIVATE_IMPL_PTR(T)(nullptr)))

#ifdef USER_API_PLUGIN_FEATURE

typedef void (__stdcall *CompilePass)(sfsl::ProgramBuilder, sfsl::Pipeline&);

#ifdef _WIN32

#include <windows.h>

typedef HINSTANCE DLLHandle;
typedef FARPROC FuncPtr;

DLLHandle loadDll(const std::string& path) {
    return LoadLibrary(path.c_str());
}

FuncPtr getSymbol(DLLHandle handle, const std::string& sym) {
    return GetProcAddress(handle, sym.c_str());
}

void unloadDll(DLLHandle handle) {
    FreeLibrary(handle);
}

#else

#include <dlfcn.h>

typedef void* DLLHandle;
typedef void* FuncPtr;

DLLHandle loadDll(const std::string& path) {
    return dlopen(path.c_str(), RTLD_LAZY);
}

FuncPtr getSymbol(DLLHandle handle, const std::string& sym) {
    FuncPtr func = dlsym(handle, sym.c_str());
    if (dlerror() != NULL) {
        return NULL;
    }
    return func;
}

void unloadDll(DLLHandle handle) {
    dlclose(handle);
}

#endif

struct Plugin final {
    Plugin(const std::string& path, DLLHandle handle, CompilePass compilePass)
        : path(path), handle(handle), compilePass(compilePass) { }

    std::string path;
    DLLHandle handle;
    CompilePass compilePass;
};

#endif /* USER_API_PLUGIN_FEATURE */

BEGIN_PRIVATE_DEF

class COMPILER_IMPL_NAME final {
public:
    COMPILER_IMPL_NAME(CompCtx_Ptr ctx, common::AbstractPrimitiveNamer* namer)
        : ctx(ctx), namer(namer) {}


#ifdef USER_API_PLUGIN_FEATURE

    ~COMPILER_IMPL_NAME() {
        unloadPlugins();
    }

    void loadPlugin(const std::string& path) {
        if (DLLHandle dll = loadDll(path)) {
            if (CompilePass cp = (CompilePass)getSymbol(dll, "compilePass")) {
                plugins.push_back(Plugin(path, dll, cp));
            } else {
                throw CompileError("Could not fetch symbol `compilePass` in plugin `" + path + "`");
            }
        } else {
            throw CompileError("Could not open plugin `" + path + "`");
        }
    }

    void unloadPlugin(const std::string& path) {
        for (auto i = plugins.begin(), e = plugins.end(); i != e; ++i) {
            if (i->path == path) {
                unloadDll(i->handle);
                plugins.erase(i);
                return;
            }
        }
    }

    void compilePass(ProgramBuilder progbuilder, Pipeline& pipeline) {
        for (const Plugin& plugin : plugins) {
            plugin.compilePass(progbuilder, pipeline);
        }
    }

    void unloadPlugins() {
        for (Plugin plugin : plugins) {
            unloadDll(plugin.handle);
        }
        plugins.clear();
    }

    std::vector<Plugin> plugins;
#else

    ~COMPILER_IMPL_NAME() {

    }

    void throwPluginNotSupportedError() {
        throw CompileError("Plugins are not supported in this version of sfsl");
    }

    void loadPlugin(const std::string&) {
        throwPluginNotSupportedError();
    }

    void unloadPlugin(const std::string&) {
        throwPluginNotSupportedError();
    }

    void compilePass(ProgramBuilder, Pipeline&) {
        throwPluginNotSupportedError();
    }

    void unloadPlugins() {
        throwPluginNotSupportedError();
    }

#endif

    CompCtx_Ptr ctx;
    common::AbstractPrimitiveNamer* namer;

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
        if (!defType) {
            throw CompileError("Type of definition was not valid");
        }

        ast::Identifier* nameId = mngr.New<ast::Identifier>(defName);
        _ddecls.push_back(mngr.New<ast::DefineDecl>(nameId, defType._impl->_type, nullptr, false, true, false));
    }

    void typeDef(const std::string& name, Type type) {
        if (!type) {
            throw CompileError("Type to typedef was not valid");
        }

        _tdecls.push_back(mngr.New<ast::TypeDecl>(mngr.New<ast::TypeIdentifier>(name), type._impl->_type));
    }

    common::AbstractMemoryManager& mngr;

    const std::string _name;
    std::vector<ast::TypeDecl*> _tdecls;
    std::vector<ast::DefineDecl*> _ddecls;
};

class PROGRAMBUILDER_IMPL_NAME final : public ModuleContainer {
public:
    PROGRAMBUILDER_IMPL_NAME(COMPILER_IMPL_PTR cmp, ast::Program* prog)
        : cmp(cmp), mngr(cmp->ctx->memoryManager()), _prog(prog) { }

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

    COMPILER_IMPL_PTR cmp;
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
        if (!fieldType) {
            throw CompileError("Type of field was not valid");
        }

        ast::Identifier* id = _mngr.New<ast::Identifier>(fieldName);
        ast::TypeExpression* tpe = fieldType._impl->_type;
        _fields.push_back(_mngr.New<ast::TypeSpecifier>(id, tpe));
    }

    void addDef(const std::string& defName, Type defType, bool isRedef, bool isExtern, bool isAbstract) {
        if (!defType) {
            throw CompileError("Type of definition was not valid");
        }

        ast::Identifier* id = _mngr.New<ast::Identifier>(defName);
        ast::TypeExpression* tpe = defType._impl->_type;
        _defs.push_back(_mngr.New<ast::DefineDecl>(id, tpe, nullptr, isRedef, isExtern, isAbstract));
    }

    Type build() const {
        if (ast::ClassDecl* clss = _mngr.New<ast::ClassDecl>(_name, nullptr,
                                                             std::vector<ast::TypeDecl*>(),
                                                             _fields, _defs, _isAbstract)) {
            return Type(NEW_TYPE_IMPL(clss));
        } else {
            return MAKE_INVALID(Type);
        }
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
            if (arg) {
                _args.push_back(arg._impl->_type);
            } else {
                throw CompileError("A parameter of TypeConstructor was not valid");
            }
        }
    }

    void setRetExpr(Type ret) {
        if (ret) {
            _ret = ret._impl->_type;
        } else {
            throw CompileError("The return type set to the TypeConstructor was not valid");
        }
    }

    Type build() const {
        ast::TypeTuple* tt = _mngr.New<ast::TypeTuple>(_args);
        ast::TypeConstructorCreation* tc = _mngr.New<ast::TypeConstructorCreation>(_name, tt, _ret);

        if (tc) {
            return Type(NEW_TYPE_IMPL(tc));
        } else {
            return MAKE_INVALID(Type);
        }
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
    common::AbstractPrimitiveNamer* namer;

    if (config.getReporter() == nullptr) {
        ctx = common::CompilationContext::DefaultCompilationContext(config.getChunkSize());
    } else {
        std::unique_ptr<ReporterAdapter> rep(new ReporterAdapter(config.getReporter()));
        ctx = common::CompilationContext::CustomReporterCompilationContext(config.getChunkSize(), std::move(rep));
    }

    if (config.getPrimitiveNamer() == nullptr) {
        namer = StandartPrimitiveNamer::DefaultPrimitiveNamer;
    } else {
        namer = config.getPrimitiveNamer();
    }

    _impl = NEW_COMPILER_IMPL(ctx, namer);
}

Compiler::~Compiler() {

}

void Compiler::loadPlugin(const std::string& pathToPluginDll) {
    _impl->loadPlugin(pathToPluginDll);
}

void Compiler::unloadPlugin(const std::string& pathToPluginDll) {
    _impl->unloadPlugin(pathToPluginDll);
}

ProgramBuilder Compiler::parse(const std::string& srcName, const std::string& srcContent) {
    try {
        src::StringSource source(src::InputSourceName::make(_impl->ctx, srcName), srcContent);
        lex::Lexer lexer(_impl->ctx, source);
        ast::Parser parser(_impl->ctx, lexer, _impl->namer);
        ast::Program* program = parser.parse();

        if (_impl->ctx->reporter().getErrorCount() == 0) {
            return ProgramBuilder(NEW_PROGRAMBUILDER_IMPL(_impl, program));
        } else {
            return MAKE_INVALID(ProgramBuilder);
        }
    } catch (const common::CompilationFatalError& err) {
        throw CompileError(err.what());
    }
}

void Compiler::compile(ProgramBuilder progBuilder, AbstractOutputCollector& collector, const Pipeline& tmp) {
    if (!progBuilder) {
        return;
    }

    // use a copy of the pipeline
    Pipeline ppl(tmp);

    _impl->compilePass(progBuilder, ppl);

    PhaseContext pctx;
    CompCtx_Ptr ctx = _impl->ctx;
    ast::Program* prog = progBuilder._impl->createUpdatedProgram();
    common::AbstractPrimitiveNamer* namer = _impl->namer;

    pctx.output("ctx", &ctx);
    pctx.output("prog", prog);
    pctx.output("namer", namer);

    // make the program builder invalid so that it can't be compiled again
    progBuilder._impl = nullptr;

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
    return MAKE_INVALID(Module);
}

Type ProgramBuilder::parseType(const std::string& str) {
    if (!_impl) {
        return MAKE_INVALID(Type);
    }

    src::StringSource source(src::InputSourceName::make(_impl->cmp->ctx, "type"), str);
    lex::Lexer lexer(_impl->cmp->ctx, source);
    ast::Parser parser(_impl->cmp->ctx, lexer, _impl->cmp->namer);
    ast::TypeExpression* tpe = parser.parseType();

    return Type(NEW_TYPE_IMPL(_impl->cmp->ctx->reporter().getErrorCount() == 0 ? tpe : nullptr));
}

Type ProgramBuilder::createFunctionType(const std::vector<Type>& argTypes, Type retType) {
    bool ok = _impl && retType;
    for (Type t : argTypes) {
        ok = ok && t;
    }

    if (!ok) {
        return MAKE_INVALID(Type);
    }

    std::vector<ast::TypeExpression*> argTypeExprs(argTypes.size());
    ast::TypeExpression* retTypeExpr = retType._impl->_type;

    std::transform(argTypes.begin(), argTypes.end(), argTypeExprs.begin(), [](Type t) { return t._impl->_type; });

    return Type(NEW_TYPE_IMPL(ast::FunctionTypeDecl::make(
                                  argTypeExprs, retTypeExpr,
                                  _impl->cmp->namer->Func(argTypes.size()),
                                  _impl->cmp->ctx)));
}

ClassBuilder ProgramBuilder::classBuilder(const std::string& className) {
    if (_impl) {
        return ClassBuilder(NEW_CLASSBUILDER_IMPL(_impl->cmp->ctx->memoryManager(), className));
    } else {
        return MAKE_INVALID(ClassBuilder);
    }
}

TypeConstructorBuilder ProgramBuilder::typeConstructorBuilder(const std::string& typeConstructorName) {
    if (_impl) {
        return TypeConstructorBuilder(NEW_TCBUILDER_IMPL(_impl->cmp->ctx->memoryManager(), typeConstructorName));
    } else {
        return MAKE_INVALID(TypeConstructorBuilder);
    }
}

// CLASS BUILDER

ClassBuilder::ClassBuilder(CLASSBUILDER_IMPL_PTR impl) : _impl(impl) {

}

ClassBuilder::~ClassBuilder() {

}

ClassBuilder::operator bool() const {
    return _impl != nullptr;
}

ClassBuilder& ClassBuilder::setAbstract(bool value) {
    if (_impl) {
        _impl->setAbstract(value);
    }
    return *this;
}

ClassBuilder& ClassBuilder::addField(const std::string& fieldName, Type fieldType) {
    if (_impl) {
        _impl->addField(fieldName, fieldType);
    }
    return *this;
}

ClassBuilder& ClassBuilder::addExternDef(const std::string& defName, Type defType, bool isRedef) {
    if (_impl) {
        _impl->addDef(defName, defType, isRedef, true, false);
    }
    return *this;
}

ClassBuilder& ClassBuilder::addAbstractDef(const std::string& defName, Type defType) {
    if (_impl) {
        _impl->addDef(defName, defType, false, false, true);
    }
    return *this;
}

Type ClassBuilder::build() const {
    if (_impl) {
        return _impl->build();
    } else {
        return MAKE_INVALID(Type);
    }
}

// TYPE CONSTRUCTOR BUILDER

TypeConstructorBuilder::TypeConstructorBuilder(TCBUILDER_IMPL_PTR impl) : _impl(impl) {

}

TypeConstructorBuilder::~TypeConstructorBuilder() {

}

TypeConstructorBuilder::operator bool() const {
    return _impl != nullptr;
}

TypeConstructorBuilder& TypeConstructorBuilder::setArgs(const std::vector<Type>& args) {
    if (_impl) {
        _impl->setArgs(args);
    }
    return *this;
}

TypeConstructorBuilder& TypeConstructorBuilder::setReturn(Type retExpr) {
    if (_impl) {
        _impl->setRetExpr(retExpr);
    }
    return *this;
}

Type TypeConstructorBuilder::build() const {
    if (_impl) {
        return _impl->build();
    } else {
        return MAKE_INVALID(Type);
    }
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
    return MAKE_INVALID(Module);
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
    return _impl != nullptr;
}

}
