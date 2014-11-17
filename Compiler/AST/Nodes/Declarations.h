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

namespace sfsl {

namespace ast {

class DefineDecl;

class ModuleDecl : public ASTNode {
public:

    ModuleDecl(Identifier* name, const std::vector<ASTNode*>& decls);
    virtual ~ModuleDecl();

    SFSL_AST_ON_VISIT_H

    const std::vector<ASTNode*>& getDeclarations() const;
    Identifier* getName() const;

private:

    Identifier* _name;
    const std::vector<ASTNode*> _decls;

};

class DefineDecl : public ASTNode {
public:

    DefineDecl(Identifier* name, ASTNode* value);
    virtual ~DefineDecl();

    SFSL_AST_ON_VISIT_H

    Identifier* getName() const;
    ASTNode* getValue() const;

private:

    Identifier* _name;
    ASTNode* _value;

};

}

}

#endif
