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
    : _scope(prog->getScope()), _namer(namer), _ctx(ctx) {

    _unitType   = createTypeFromSymbol(getSymbol(_namer->Unit()));
    _boolType   = createTypeFromSymbol(getSymbol(_namer->Bool()));
    _intType    = createTypeFromSymbol(getSymbol(_namer->Int()));
    _realType   = createTypeFromSymbol(getSymbol(_namer->Real()));
    _boxType    = createTypeFromSymbol(getSymbol(_namer->Box()));
    _stringType = createTypeFromSymbol(getSymbol(_namer->String()));
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

type::Type* SymbolResolver::Box() const {
    if (!_boxType) {
        throw common::CompilationFatalError("Box type was not set");
    }
    return _boxType;
}

type::Type* SymbolResolver::Func(size_t nbArgs) const {
    if (nbArgs >= _funcTypes.size()) {
        for (size_t i = _funcTypes.size(); i <= nbArgs; ++i) {
            _funcTypes.push_back(createTypeFromSymbol(getSymbol(_namer->Func(i))));
        }
    }

    if (!_funcTypes[nbArgs]) {
        throw common::CompilationFatalError("Func" + utils::T_toString(nbArgs) + " type was not set");
    }
    return _funcTypes[nbArgs];
}

type::Type* SymbolResolver::createTypeFromSymbol(Symbol* sym) const {
    if (sym) {
        if (sym::TypeSymbol* tpesym = sym::getIfSymbolOfType<sym::TypeSymbol>(sym)) {
            return ast::ASTTypeCreator::createType(tpesym->getTypeDecl()->getExpression(), _ctx);
            /*return _ctx->memoryManager().New<type::ProperType>(
                        ast::getClassDeclFromTypeSymbol(static_cast<sym::TypeSymbol*>(sym), _ctx));*/
        }
    }
    throw common::CompilationFatalError("Cannot create type : " +
                                        (sym ? (sym->getName() + " is not a type") : "null pointer was passed"));
}

}

}
