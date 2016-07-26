//
//  ASTExpr2TypeExpr.h
//  SFSL
//
//  Created by Romain Beguet on 26.07.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ASTExpr2TypeExpr__
#define __SFSL__ASTExpr2TypeExpr__

#include "ASTVisitor.h"
#include "ASTTransformer.h"

namespace sfsl {

namespace ast {

class ASTExpr2TypeExpr : public ASTTransformer {
public:

    ASTExpr2TypeExpr(CompCtx_Ptr& ctx);

    virtual ~ASTExpr2TypeExpr();

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

    /**
     * @brief Transform an Expression tree in its equivalent
     * TypeExpression tree. For example, the expression:
     * `a.b` represented by MemberAccess(Identifier, Identifier)
     * will be transformed to TypeMemberAccess(TypeIdentifier, TypeIdentifier)
     *
     * @param node The representing containing the expression
     * @param ctx The compilation context
     * @return The generated equivalent type expression (or nullptr
     * if it was not convertible)
     */
    static TypeExpression* convert(Expression* node, CompCtx_Ptr& ctx);
};

}

}

#endif
