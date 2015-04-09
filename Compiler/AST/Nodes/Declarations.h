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
#include "Statements.h"
#include "../Symbols/Symbols.h"
#include "../Symbols/Symbolic.h"

namespace sfsl {

namespace ast {

class DefineDecl;
class ClassDecl;

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
               const std::vector<ClassDecl*>& classes,
               const std::vector<DefineDecl*>& decls);
    virtual ~ModuleDecl();

    SFSL_AST_ON_VISIT_H

    /**
     * @return All the sub modules declarations that were made inside this module
     */
    const std::vector<ModuleDecl*>& getSubModules() const;

    /**
     * @return All the class definitions that were made inside this module
     */
    const std::vector<ClassDecl*>& getClasses() const;

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
    const std::vector<ClassDecl*> _classes;
    const std::vector<DefineDecl*> _decls;
};

/**
 * @brief The Define Declaration AST
 * Contains :
 *  - This definition's name
 *  - The value associated to this definition
 */
class DefineDecl : public Statement, public sym::Symbolic<sym::DefinitionSymbol> {
public:

    DefineDecl(Identifier* name, ASTNode* value);
    virtual ~DefineDecl();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The name of this definition
     */
    Identifier* getName() const;

    /**
     * @return The value associated to this definition
     */
    ASTNode* getValue() const;

private:

    Identifier* _name;
    ASTNode* _value;

};

/**
 * @brief The Class Declaration AST
 * Contains :
 *  - The class name
 *  - Its fields
 *  - Its definitions
 */
class ClassDecl : public Statement, public sym::Symbolic<sym::ClassSymbol> {
public:

    ClassDecl(Identifier* name, const std::vector<TypeSpecifier*> fields, const std::vector<DefineDecl*> defs);
    virtual ~ClassDecl();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The name of the class
     */
    Identifier* getName() const;

    /**
     * @return The list of fields declared in this class
     */
    const std::vector<TypeSpecifier*>& getFields() const;

    /**
     * @return The list of definitions declared in this class
     */
    const std::vector<DefineDecl*>& getDefs() const;

private:

    Identifier* _name;
    std::vector<TypeSpecifier*> _fields;
    std::vector<DefineDecl*> _defs;
};

}

}

#endif
