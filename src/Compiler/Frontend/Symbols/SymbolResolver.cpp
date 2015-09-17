//
//  SymbolResolver.cpp
//  SFSL
//
//  Created by Romain Beguet on 14.04.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "SymbolResolver.h"
#include "../AST/Visitors/ASTTypeCreator.h"
#include "Scope.h"

namespace sfsl {

namespace sym {

SymbolResolver::SymbolResolver(const ast::Program* prog, const CompCtx_Ptr &ctx)
    : _scope(prog->getScope()), _ctx(ctx) {

}

SymbolResolver::~SymbolResolver() {

}

Symbol* SymbolResolver::getSymbol(const std::string& fullPathName) const {
    std::vector<std::string> parts;
    size_t i = 0, e = utils::split(parts, fullPathName, NAMESPACE_DELIMITER);
    Scope* scope = _scope;
    Symbol* lastSym = nullptr;

    for (; i < e; ++i) {
        if (!scope) {
            return nullptr;
        }

        if ((lastSym = scope->getSymbol<sym::Symbol>(parts[i], false))) {
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

void SymbolResolver::setPredefClassesPath(const std::string& fullPathName) {
    _unitType   = createTypeFromSymbol(getSymbol(fullPathName + NAMESPACE_DELIMITER + UNIT_CLASS_NAME));
    _boolType   = createTypeFromSymbol(getSymbol(fullPathName + NAMESPACE_DELIMITER + BOOL_CLASS_NAME));
    _intType    = createTypeFromSymbol(getSymbol(fullPathName + NAMESPACE_DELIMITER + INT_CLASS_NAME));
    _realType   = createTypeFromSymbol(getSymbol(fullPathName + NAMESPACE_DELIMITER + REAL_CLASS_NAME));
    _stringType = createTypeFromSymbol(getSymbol(fullPathName + NAMESPACE_DELIMITER + STRING_CLASS_NAME));
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

type::Type* SymbolResolver::createTypeFromSymbol(Symbol* sym) {
    if (sym) {
        if (sym->getSymbolType() == SYM_TPE) {
            return _ctx->memoryManager().New<type::ProperType>(
                        ast::getClassDeclFromTypeSymbol(static_cast<sym::TypeSymbol*>(sym), _ctx));
        }
    }
    throw common::CompilationFatalError("Cannot create type : " +
                                        (sym ? (sym->getName() + " is not a type") : "null pointer was passed"));
}

}

}
