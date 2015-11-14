//
//  ASTExplicitVisitor.h
//  SFSL
//
//  Created by Romain Beguet on 26.08.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ASTExplicitVisitor__
#define __SFSL__ASTExplicitVisitor__

#include "ASTVisitor.h"

namespace sfsl {

namespace ast {

class ASTExplicitVisitor : public ASTVisitor {
public:

    ASTExplicitVisitor(CompCtx_Ptr& ctx);

    virtual ~ASTExplicitVisitor();

    virtual void visit(ASTNode*) override;

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
    virtual void visit(KindSpecifier* ks) override;

    virtual void visit(ExpressionStatement* exp) override;

    virtual void visit(AssignmentExpression* aex) override;
    virtual void visit(TypeSpecifier* tps) override;
    virtual void visit(Block* block) override;
    virtual void visit(IfExpression* ifexpr) override;
    virtual void visit(MemberAccess* dot) override;
    virtual void visit(Tuple* tuple) override;
    virtual void visit(FunctionCreation* func) override;
    virtual void visit(FunctionCall* call) override;
    virtual void visit(Identifier* ident) override;
    virtual void visit(This* ths) override;
    virtual void visit(BoolLitteral* boollit) override;
    virtual void visit(IntLitteral* intlit) override;
    virtual void visit(RealLitteral* reallit) override;
    virtual void visit(StringLitteral* strlit) override;

};

}

}

#endif
