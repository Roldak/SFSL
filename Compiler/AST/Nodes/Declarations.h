//
//  Declarations.h
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Declarations__
#define __SFSL__Declarations__

#include <iostream>
#include <vector>
#include "Expressions.h"
#include "TypeExpressions.h"
#include "../Symbols/Symbols.h"
#include "../Symbols/Symbolic.h"

namespace sfsl {

namespace ast {

class DefineDecl;
class TypeDecl;

/**
 * @brief The Module AST
 * Contains :
 *  - This module's name
 *  - The list of its submodules
 *  - The list of its classes
 *  - The list of its definitions
 */
class ModuleDecl : public ASTNode, public sym::Symbolic<sym::ModuleSymbol> {
public:

    ModuleDecl(Identifier* name,
               const std::vector<ModuleDecl*>& mods,
               const std::vector<TypeDecl*>& classes,
               const std::vector<DefineDecl*>& decls);
    virtual ~ModuleDecl();

    SFSL_AST_ON_VISIT_H

    /**
     * @return All the sub modules declarations that were made inside this module
     */
    const std::vector<ModuleDecl*>& getSubModules() const;

    /**
     * @return All the type definitions that were made inside this module
     */
    const std::vector<TypeDecl*>& getTypes() const;

    /**
     * @return All the declarations that were made inside this module
     */
    const std::vector<DefineDecl*>& getDeclarations() const;

    /**
     * @return The name of this module
     */
    Identifier* getName() const;

private:

    Identifier* _name;
    const std::vector<ModuleDecl*> _mods;
    const std::vector<TypeDecl*> _types;
    const std::vector<DefineDecl*> _decls;
};

/**
 * @brief The Define Declaration AST
 * Contains :
 *  - This definition's name
 *  - The value associated to this definition
 */
class DefineDecl : public Expression, public sym::Symbolic<sym::DefinitionSymbol> {
public:

    DefineDecl(Identifier* name, Expression* value);
    virtual ~DefineDecl();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The name of this definition
     */
    Identifier* getName() const;

    /**
     * @return The value associated to this definition
     */
    Expression* getValue() const;

private:

    Identifier* _name;
    Expression* _value;

};

class TypeDecl : public Expression, public sym::Symbolic<sym::TypeSymbol> {
public:

    TypeDecl(Identifier* id, Expression* exp);
    virtual ~TypeDecl();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The name of the type
     */
    Identifier* getName() const;
    
    /**
     * @return The type expression
     */
    Expression* getExpression() const;

private:

    Identifier* _name;
    Expression* _exp;
};

}

}

#endif
