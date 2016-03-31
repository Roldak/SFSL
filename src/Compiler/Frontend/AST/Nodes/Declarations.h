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
#include "KindExpressions.h"

#include "../../Symbols/Symbols.h"
#include "../../Symbols/Symbolic.h"

#include "../Utils/CanUseModules.h"
#include "../Utils/Annotations.h"

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
class ModuleDecl :
        public ASTNode,
        public sym::Symbolic<sym::ModuleSymbol>,
        public CanUseModules,
        public Annotable {
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
class DefineDecl :
        public Expression,
        public sym::Symbolic<sym::DefinitionSymbol>,
        public Annotable {
public:

    DefineDecl(Identifier* name, TypeExpression* tp, Expression* value, bool isRedef, bool isExtern, bool isAbstract);
    virtual ~DefineDecl();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The name of this definition
     */
    Identifier* getName() const;

    /**
     * @return The type specifying expression (nullptr if none were specified)
     */
    TypeExpression* getTypeSpecifier() const;

    /**
     * @return The value associated to this definition
     */
    Expression* getValue() const;

    /**
     * @return True if the definition is supposed to override another one
     */
    bool isRedef() const;

    /**
     * @return True if the definition is marked with the `extern` flag
     */
    bool isExtern() const;

    /**
     * @return True if the definition is marked with the `abstract` flag
     */
    bool isAbstract() const;

private:

    Identifier* _name;
    TypeExpression* _typeSpecifier;
    Expression* _value;

    bool _isRedef;
    bool _isExtern;
    bool _isAbstract;

};

class TypeDecl :
        public Expression,
        public sym::Symbolic<sym::TypeSymbol>,
        public Annotable {
public:

    TypeDecl(TypeIdentifier* id, TypeExpression* exp);
    virtual ~TypeDecl();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The name of the type
     */
    TypeIdentifier* getName() const;
    
    /**
     * @return The type expression
     */
    TypeExpression* getExpression() const;

private:

    TypeIdentifier* _name;
    TypeExpression* _exp;
};

}

}

#endif
