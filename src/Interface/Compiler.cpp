//
//  Compiler.cpp
//  SFSL
//
//  Created by Romain Beguet on 18.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include <ctime>

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
#include "Compiler/Backend/AST2BAST/PreTransform.h"
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

typedef void (__stdcall *CompilePass)(sfsl::ProgramBuilder, sfsl::Pipeline&, const std::vector<std::string>&);

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
    Plugin(const std::string& path, const std::vector<std::string>& args, DLLHandle handle, CompilePass compilePass)
        : path(path), args(args), handle(handle), compilePass(compilePass) { }

    std::string path;
    std::vector<std::string> args;

    DLLHandle handle;
    CompilePass compilePass;
};

#endif /* USER_API_PLUGIN_FEATURE */

BEGIN_PRIVATE_DEF

class COMPILER_IMPL_NAME final {
public:
    COMPILER_IMPL_NAME(const CompilerConfig& config) : config(config) {
        AbstractReporter* reporter = nullptr;
        size_t initialChunkSize = 2048;

        config.get<opt::InitialChunkSize>(initialChunkSize);

        if (config.get<opt::Reporter>(reporter)) {
            ctx = common::CompilationContext::CustomReporterCompilationContext(
                        initialChunkSize,
                        std::unique_ptr<ReporterAdapter>(new ReporterAdapter(reporter)));
        } else {
            ctx = common::CompilationContext::DefaultCompilationContext(initialChunkSize);
        }

        if (!config.get<opt::PrimitiveNamer>(namer)) {
            namer = StandartPrimitiveNamer::DefaultPrimitiveNamer;
        }
    }


#ifdef USER_API_PLUGIN_FEATURE

    ~COMPILER_IMPL_NAME() {
        unloadPlugins();
    }

    void loadPlugin(const std::string& path, const std::vector<std::string>& args) {
        if (DLLHandle dll = loadDll(path)) {
            if (CompilePass cp = (CompilePass)getSymbol(dll, "compilePass")) {
                plugins.push_back(Plugin(path, args, dll, cp));
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
            plugin.compilePass(progbuilder, pipeline, plugin.args);
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

    }

    void unloadPlugins() {

    }

#endif

    CompilerConfig config;
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
        _ddecls.push_back(mngr.New<ast::DefineDecl>(nameId, defType._impl->_type, nullptr, ast::DefFlags::EXTERN));
    }

    void typeDef(const std::string& name, Type type, bool isExtern) {
        if (!type) {
            throw CompileError("Type to typedef was not valid");
        }

        _tdecls.push_back(mngr.New<ast::TypeDecl>(mngr.New<ast::TypeIdentifier>(name), type._impl->_type, isExtern));
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

            ast::Program* updatedProg = mngr.New<ast::Program>(modules);
            updatedProg->setPos(*_prog);
            return updatedProg;
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

    void addDef(const std::string& defName, Type defType, ast::DefFlags flags) {
        if (!defType) {
            throw CompileError("Type of definition was not valid");
        }

        ast::Identifier* id = _mngr.New<ast::Identifier>(defName);
        ast::TypeExpression* tpe = defType._impl->_type;
        _defs.push_back(_mngr.New<ast::DefineDecl>(id, tpe, nullptr, flags));
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

    void setParams(const std::vector<TypeConstructorBuilder::Parameter>& params) {
        _params = params;
    }

    void setRetExpr(Type ret) {
        if (ret) {
            _ret = ret._impl->_type;
        } else {
            throw CompileError("The return type set to the TypeConstructor was not valid");
        }
    }

    Type build() const {
        std::vector<ast::TypeExpression*> params(_params.size());

        for (size_t i = 0; i < params.size(); ++i) {
            common::VARIANCE_TYPE vt;
            switch (_params[i].getVarianceType()) {
            case TypeConstructorBuilder::Parameter::V_IN:   vt = common::VAR_T_IN; break;
            case TypeConstructorBuilder::Parameter::V_OUT:  vt = common::VAR_T_OUT; break;
            case TypeConstructorBuilder::Parameter::V_NONE: vt = common::VAR_T_NONE; break;
            }

            ast::TypeIdentifier* id = _mngr.New<ast::TypeIdentifier>(_params[i].getName());
            ast::KindSpecifyingExpression* kd = _mngr.New<ast::ProperTypeKindSpecifier>();

            params[i] = _mngr.New<ast::TypeParameter>(vt, id, kd);
        }

        ast::TypeTuple* tt = _mngr.New<ast::TypeTuple>(params);
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

    std::vector<TypeConstructorBuilder::Parameter> _params;
    ast::TypeExpression* _ret;
};

END_PRIVATE_DEF

namespace sfsl {

// COMPILER

Compiler::Compiler(const CompilerConfig& config) {
    _impl = NEW_COMPILER_IMPL(config);
}

Compiler::~Compiler() {

}

void Compiler::loadPlugin(const std::string& pathToPluginDll, const std::vector<std::string>& args) {
    _impl->loadPlugin(pathToPluginDll, args);
}

void Compiler::unloadPlugin(const std::string& pathToPluginDll) {
    _impl->unloadPlugin(pathToPluginDll);
}

ProgramBuilder Compiler::parse(const std::string& srcName, const std::string& srcContent) {
    try {
        common::ChunkedMemoryManager lexerMemMngr(srcContent.size() * sizeof(tok::Identifier) / 3 + 40); // random heuristic :D

        src::StringSource source(src::InputSourceName::make(_impl->ctx, srcName), srcContent);
        lex::Lexer lexer(lexerMemMngr, _impl->ctx->reporter(), source);
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
    sym::SymbolResolver res(prog, namer, ctx);

    pctx.output("ctx", &ctx);
    pctx.output("prog", prog);
    pctx.output("namer", namer);
    pctx.output("res", &res);

    // make the program builder invalid so that it can't be compiled again
    progBuilder._impl = nullptr;

    opt::AfterEachPhase::ReportingFunction afterEachPhaseRep;
    opt::AtEnd::ReportingFunction atEndRep;

    _impl->config.get<opt::AfterEachPhase>(afterEachPhaseRep);
    _impl->config.get<opt::AtEnd>(atEndRep);

    try {
        std::set<std::shared_ptr<Phase>> phases(ppl.getPhases());
        std::vector<std::shared_ptr<Phase>> sortedPhases(sortPhases(phases));

        clock_t compilationStart = clock();

        for (std::shared_ptr<Phase> phase : sortedPhases) {

            clock_t phaseStart = clock();
            bool success = phase->run(pctx);

            if (afterEachPhaseRep) {
                afterEachPhaseRep(phase->getName(), (clock() - phaseStart) / (double) CLOCKS_PER_SEC, ctx->memoryManager().getInfos());
            }

            if (!success) {
                break;
            }
        }

        if (atEndRep) {
            atEndRep((clock() - compilationStart) / (double) CLOCKS_PER_SEC, ctx->memoryManager().getInfos());
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
    lex::Lexer lexer(_impl->cmp->ctx->memoryManager(), _impl->cmp->ctx->reporter(), source);
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
                                  std::vector<ast::TypeExpression*>(),
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
        _impl->addDef(defName, defType, ast::DefFlags::EXTERN | (isRedef ? ast::DefFlags::REDEF : ast::DefFlags::NONE));
    }
    return *this;
}

ClassBuilder& ClassBuilder::addAbstractDef(const std::string& defName, Type defType) {
    if (_impl) {
        _impl->addDef(defName, defType, ast::DefFlags::ABSTRACT);
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

// TYPE PARAMETER

TypeConstructorBuilder::Parameter::Parameter(const std::string& name, TypeConstructorBuilder::Parameter::V_TYPE varType)
    : _name(name), _varType(varType) {

}

const std::string& TypeConstructorBuilder::Parameter::getName() const {
    return _name;
}

TypeConstructorBuilder::Parameter::V_TYPE TypeConstructorBuilder::Parameter::getVarianceType() const {
    return _varType;
}

// TYPE CONSTRUCTOR BUILDER

TypeConstructorBuilder::TypeConstructorBuilder(TCBUILDER_IMPL_PTR impl) : _impl(impl) {

}

TypeConstructorBuilder::~TypeConstructorBuilder() {

}

TypeConstructorBuilder::operator bool() const {
    return _impl != nullptr;
}

TypeConstructorBuilder& TypeConstructorBuilder::setParams(const std::vector<Parameter>& args) {
    if (_impl) {
        _impl->setParams(args);
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

void Module::externTypeDef(const std::string& typeName, Type type) {
    if (_impl) {
        _impl->typeDef(typeName, type, true);
    }
}

void Module::typeDef(const std::string& typeName, Type type) {
    if (_impl) {
        _impl->typeDef(typeName, type, false);
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
