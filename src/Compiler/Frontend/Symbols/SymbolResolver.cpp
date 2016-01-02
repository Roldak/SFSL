//
//  SymbolResolver.cpp
//  SFSL
//
//  Created by Romain Beguet on 14.04.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "SymbolResolver.h"
#include "../../../Common/AbstractPrimitiveNamer.h"
#include "../AST/Visitors/ASTTypeCreator.h"
#include "Scope.h"

namespace sfsl {

namespace sym {

SymbolResolver::SymbolResolver(const ast::Program* prog, const common::AbstractPrimitiveNamer* namer, const CompCtx_Ptr& ctx)
    : _scope(prog->getScope()), _ctx(ctx) {

    _unitType   = createTypeFromSymbol(getSymbol(namer->Unit()));
    _boolType   = createTypeFromSymbol(getSymbol(namer->Bool()));
    _intType    = createTypeFromSymbol(getSymbol(namer->Int()));
    _realType   = createTypeFromSymbol(getSymbol(namer->Real()));
    _stringType = createTypeFromSymbol(getSymbol(namer->String()));

    for (size_t i = 0; i < NUMBER_OF_FUNC_TYPES; ++i) {
        _funcTypes[i] = createTypeFromSymbol(getSymbol(namer->Func(i)));
    }
}

SymbolResolver::~SymbolResolver() {

}

Symbol* SymbolResolver::getSymbol(const std::vector<std::string>& fullPath) const {
    Scope* scope = _scope;
    Symbol* lastSym = nullptr;

    for (const std::string& part : fullPath) {
        if (!scope) {
            return nullptr;
        }

        if ((lastSym = scope->getSymbol<sym::Symbol>(part, false))) {
            Scoped* scoped;

            switch (lastSym->getSymbolType()) {
                case SYM_TPE:       scoped = ast::getClassDeclFromTypeSymbol(static_cast<TypeSymbol*>(lastSym), _ctx); break;
                case SYM_MODULE:    scoped = static_cast<ModuleSymbol*>(lastSym); break;
                default:            scoped = nullptr; break;
            }

            scope = scoped ? scoped->getScope() : nullptr;
        } else {
            return nullptr;
        }
    }

    return lastSym;
}

type::Type* SymbolResolver::Unit() const {
    if (!_unitType) {
        throw common::CompilationFatalError("Unit type was not set");
    }
    return _unitType;
}

type::Type* SymbolResolver::Bool() const {
    if (!_boolType) {
        throw common::CompilationFatalError("Bool type was not set");
    }
    return _boolType;
}

type::Type* SymbolResolver::Int() const {
    if (!_intType) {
        throw common::CompilationFatalError("Int type was not set");
    }
    return _intType;
}

type::Type* SymbolResolver::Real() const {
    if (!_realType) {
        throw common::CompilationFatalError("Real type was not set");
    }
    return _realType;
}

type::Type* SymbolResolver::String() const {
    if (!_stringType) {
        throw common::CompilationFatalError("String type was not set");
    }
    return _stringType;
}

type::Type* SymbolResolver::Func(size_t nbArgs) const {
    if (nbArgs >= NUMBER_OF_FUNC_TYPES) {
        throw common::CompilationFatalError("Func" + utils::T_toString(nbArgs) + " does not exists");
    }

    if (!_funcTypes[nbArgs]) {
        throw common::CompilationFatalError("Func" + utils::T_toString(nbArgs) + " type was not set");
    }
    return _funcTypes[nbArgs];
}

type::Type* SymbolResolver::createTypeFromSymbol(Symbol* sym) {
    if (sym) {
        if (sym->getSymbolType() == SYM_TPE) {
            return ast::ASTTypeCreator::createType(static_cast<sym::TypeSymbol*>(sym)->getTypeDecl()->getExpression(), _ctx);
            /*return _ctx->memoryManager().New<type::ProperType>(
                        ast::getClassDeclFromTypeSymbol(static_cast<sym::TypeSymbol*>(sym), _ctx));*/
        }
    }
    throw common::CompilationFatalError("Cannot create type : " +
                                        (sym ? (sym->getName() + " is not a type") : "null pointer was passed"));
}

}

}
