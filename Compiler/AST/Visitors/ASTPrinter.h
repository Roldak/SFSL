//
//  ASTPrinter.h
//  SFSL
//
//  Created by Romain Beguet on 17.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ASTPrinter__
#define __SFSL__ASTPrinter__

#include <iostream>
#include "ASTVisitor.h"

namespace sfsl {

namespace ast {

/**
 * @brief An ASTVisitor that prints the AST into a readable form.
 * (parsing its output again should give the same AST as the original one)
 */
class ASTPrinter : public ASTVisitor {
public:

    ASTPrinter(std::shared_ptr<common::CompilationContext>& ctx);

    virtual void visit(ModuleDecl* module);

    virtual void visit(DefineDecl* decl);

    virtual void visit(BinaryExpression* exp);
    virtual void visit(FunctionCall* call);
    virtual void visit(Identifier* ident);
    virtual void visit(IntLitteral* intlit);
    virtual void visit(RealLitteral* reallit);
};

}

}

#endif
