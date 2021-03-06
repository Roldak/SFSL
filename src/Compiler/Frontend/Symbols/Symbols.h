//
//  Symbols.h
//  SFSL
//
//  Created by Romain Beguet on 20.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Symbols__
#define __SFSL__Symbols__

#include <iostream>
#include <map>
#include "../../Common/MemoryManageable.h"
#include "../../Common/Positionnable.h"
#include "../../Common/ManageableUserData.h"
#include "../Types/Types.h"
#include "../Kinds/Kinds.h"
#include "../AST/Utils/UsageTrackable.h"
#include "Scoped.h"

namespace sfsl {

namespace ast {
class TypeExpression;
class TypeDecl;
class DefineDecl;
}

namespace sym {

enum SYM_TYPE{SYM_MODULE = 0, SYM_TPE, SYM_DEF, SYM_VAR};

class ModuleSymbol;
class TypeSymbol;
class DefinitionSymbol;
class VariableSymbol;

/**
 * @brief Abstract class representing a Symbol, which is a uniquely named object
 */
class Symbol : public common::MemoryManageable, public common::Positionnable {
public:
    virtual ~Symbol();

    /**
     * @return The type of this symbol
     */
    virtual SYM_TYPE getSymbolType() const = 0;

    /**
     * @return True if this symbol is overloadable with the given one
     */
    virtual bool isOverloadableWith(sym::Symbol* other) const;

    /**
     * @return The name of this symbol
     */
    const std::string& getName() const;

    /**
     * @return The absolute name of this symbol (path.name)
     */
    const std::string& getAbsoluteName() const;

protected:
    Symbol(const std::string& name, const std::string& absoluteName);

private:

    const std::string _name;
    const std::string _absoluteName;
};

/**
 * @brief Represents the symbol associated to a module.
 */
class ModuleSymbol : public Symbol, public Scoped {
public:
    ModuleSymbol(const std::string& name, const std::string& absoluteName);
    virtual ~ModuleSymbol();

    virtual SYM_TYPE getSymbolType() const override;
};

/**
 * @brief Represents the symbol associated to a type
 */
class TypeSymbol : public Symbol, public Scoped, public type::Typed, public kind::Kinded, public common::HasManageableUserdata {
public:
    TypeSymbol(const std::string& name, const std::string& absoluteName, ast::TypeDecl* type);
    virtual ~TypeSymbol();

    virtual SYM_TYPE getSymbolType() const override;

    ast::TypeDecl* getTypeDecl() const;

private:

    ast::TypeDecl* _type;
};

/**
 * @brief Represents the symbol associated to a definition
 */
class DefinitionSymbol : public Symbol, public Scoped, public type::Typed, public common::HasManageableUserdata {
public:
    DefinitionSymbol(const std::string& name, const std::string& absoluteName, ast::DefineDecl* def, ast::TypeExpression* owner = nullptr);
    virtual ~DefinitionSymbol();

    virtual SYM_TYPE getSymbolType() const override;

    virtual bool isOverloadableWith(sym::Symbol* other) const override;

    ast::DefineDecl* getDef() const;
    ast::TypeExpression* getOwner() const;

    void setOverridenSymbol(DefinitionSymbol* def);
    DefinitionSymbol* getOverridenSymbol() const;

private:

    ast::DefineDecl* _def;
    ast::TypeExpression* _owner;

    DefinitionSymbol* _overriden;
};

/**
 * @brief Represents the symbol associated to a variable
 */
class VariableSymbol :
        public Symbol,
        public type::Typed,
        public ast::UsageTrackable,
        public common::HasManageableUserdata {
public:
    VariableSymbol(const std::string& name, const std::string& absoluteName);
    virtual ~VariableSymbol();

    virtual SYM_TYPE getSymbolType() const override;
};

struct SymbolData final {
    SymbolData(sym::Symbol* s, const type::Environment& e);

    sym::Symbol* symbol;
    type::Environment env;
};

template<typename T>
inline T* getIfSymbolOfType(Symbol* s) {
    return nullptr;
}

template<>
inline ModuleSymbol* getIfSymbolOfType(Symbol* s) {
    return (s->getSymbolType() == SYM_MODULE) ? static_cast<ModuleSymbol*>(s) : nullptr;
}

template<>
inline TypeSymbol* getIfSymbolOfType(Symbol* s) {
    return (s->getSymbolType() == SYM_TPE) ? static_cast<TypeSymbol*>(s) : nullptr;
}

template<>
inline DefinitionSymbol* getIfSymbolOfType(Symbol* s) {
    return (s->getSymbolType() == SYM_DEF) ? static_cast<DefinitionSymbol*>(s) : nullptr;
}

template<>
inline VariableSymbol* getIfSymbolOfType(Symbol* s) {
    return (s->getSymbolType() == SYM_VAR) ? static_cast<VariableSymbol*>(s) : nullptr;
}

}

}

#endif
