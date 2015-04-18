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

    ASTPrinter(CompCtx_Ptr& ctx);

    virtual ~ASTPrinter();

    virtual void visit(ModuleDecl* module);
    virtual void visit(TypeDecl* tdecl);
    virtual void visit(ClassDecl* clss);
    virtual void visit(DefineDecl* decl);

    virtual void visit(TypeTuple* ttuple);
    virtual void visit(TypeConstructorCreation* typeconstructor);

    virtual void visit(ExpressionStatement* exp);

    virtual void visit(BinaryExpression* exp);
    virtual void visit(AssignmentExpression* aex);
    virtual void visit(TypeSpecifier* tps);
    virtual void visit(Block* block);
    virtual void visit(IfExpression* ifexpr);
    virtual void visit(MemberAccess* dot);
    virtual void visit(Tuple* tuple);
    virtual void visit(FunctionCreation* func);
    virtual void visit(Identifier* ident);
    virtual void visit(IntLitteral* intlit);
    virtual void visit(RealLitteral* reallit);

private :

    void printIndents();

    size_t _indentCount;
};

}

}

#endif
