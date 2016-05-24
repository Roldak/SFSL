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

    ASTPrinter(CompCtx_Ptr& ctx, std::ostream& ostream);

    virtual ~ASTPrinter();

    virtual void visit(ASTNode* node) override;

    virtual void visit(Program* prog) override;
    virtual void visit(ModuleDecl* module) override;
    virtual void visit(TypeDecl* tdecl) override;
    virtual void visit(ClassDecl* clss) override;
    virtual void visit(DefineDecl* decl) override;

    virtual void visit(ProperTypeKindSpecifier* ptks) override;
    virtual void visit(TypeConstructorKindSpecifier* tcks) override;

    virtual void visit(FunctionTypeDecl* ftdecl) override;
    virtual void visit(TypeMemberAccess* tdot) override;
    virtual void visit(TypeTuple* ttuple) override;
    virtual void visit(TypeConstructorCreation* typeconstructor) override;
    virtual void visit(TypeConstructorCall* tcall) override;
    virtual void visit(TypeIdentifier* tident) override;
    virtual void visit(TypeToBeInferred* tbi) override;
    virtual void visit(TypeParameter* tparam) override;

    virtual void visit(ExpressionStatement* exp) override;

    virtual void visit(AssignmentExpression* aex) override;
    virtual void visit(TypeSpecifier* tps) override;
    virtual void visit(Block* block) override;
    virtual void visit(IfExpression* ifexpr) override;
    virtual void visit(MemberAccess* dot) override;
    virtual void visit(Tuple* tuple) override;
    virtual void visit(FunctionCreation* func) override;
    virtual void visit(FunctionCall* call) override;
    virtual void visit(Instantiation* inst) override;
    virtual void visit(Identifier* ident) override;
    virtual void visit(This* ths) override;
    virtual void visit(BoolLiteral* boollit) override;
    virtual void visit(IntLiteral* intlit) override;
    virtual void visit(RealLiteral* reallit) override;
    virtual void visit(StringLiteral* strlit) override;

private :

    void printIndents();
    void printUsings(const CanUseModules& canUseModules, bool asStatement);

    size_t _indentCount;

    std::ostream& _ostream;
};

}

}

#endif
