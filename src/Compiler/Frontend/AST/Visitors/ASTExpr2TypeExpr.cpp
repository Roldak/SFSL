//
//  ASTExpr2TypeExpr.h
//  SFSL
//
//  Created by Romain Beguet on 26.07.16
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#include "ASTExpr2TypeExpr.h"
#include "../../../Common/CompilationContext.h"

#define ERROR_FOR_NODE(nodeType) \
    void ASTExpr2TypeExpr::visit(nodeType* n) { \
        _created = nullptr; \
        return; \
    }

#define IDENTITY_FOR_NODE(nodeType) \
    void ASTExpr2TypeExpr::visit(nodeType* n) { \
        _created = n; \
        return; \
    }

namespace sfsl {

namespace ast {

ASTExpr2TypeExpr::ASTExpr2TypeExpr(CompCtx_Ptr& ctx) : ASTTransformer(ctx) {

}

ASTExpr2TypeExpr::~ASTExpr2TypeExpr() {

}

ERROR_FOR_NODE(ASTNode)
ERROR_FOR_NODE(Program)
ERROR_FOR_NODE(ModuleDecl)
ERROR_FOR_NODE(TypeDecl)
ERROR_FOR_NODE(DefineDecl)
ERROR_FOR_NODE(ProperTypeKindSpecifier)
ERROR_FOR_NODE(TypeConstructorKindSpecifier)
ERROR_FOR_NODE(ExpressionStatement)
ERROR_FOR_NODE(AssignmentExpression)
ERROR_FOR_NODE(TypeSpecifier)
ERROR_FOR_NODE(Block)
ERROR_FOR_NODE(IfExpression)
ERROR_FOR_NODE(Tuple)
ERROR_FOR_NODE(FunctionCreation)
ERROR_FOR_NODE(FunctionCall)
ERROR_FOR_NODE(Instantiation)
ERROR_FOR_NODE(This)
ERROR_FOR_NODE(BoolLiteral)
ERROR_FOR_NODE(IntLiteral)
ERROR_FOR_NODE(RealLiteral)
ERROR_FOR_NODE(StringLiteral)

IDENTITY_FOR_NODE(ClassDecl)
IDENTITY_FOR_NODE(FunctionTypeDecl)
IDENTITY_FOR_NODE(TypeMemberAccess)
IDENTITY_FOR_NODE(TypeTuple)
IDENTITY_FOR_NODE(TypeConstructorCreation)
IDENTITY_FOR_NODE(TypeConstructorCall)
IDENTITY_FOR_NODE(TypeIdentifier)
IDENTITY_FOR_NODE(TypeToBeInferred)
IDENTITY_FOR_NODE(TypeParameter)

void ASTExpr2TypeExpr::visit(MemberAccess* dot) {
    if (TypeExpression* accessed = transform<TypeExpression>(dot->getAccessed())) {
        if (TypeIdentifier* member = transform<TypeIdentifier>(dot->getMember())) {
            TypeMemberAccess* tdot = make<TypeMemberAccess>(accessed, member);
            tdot->setSymbol(dot->getSymbol());
            tdot->setPos(*dot);
            return;
        }
    }
    _created = nullptr;
}
void ASTExpr2TypeExpr::visit(Identifier* ident) {
    TypeIdentifier* tident = make<TypeIdentifier>(ident->getValue());
    tident->setSymbol(ident->getSymbol());
    tident->setPos(*ident);
}

TypeExpression* ASTExpr2TypeExpr::convert(Expression* node, CompCtx_Ptr& ctx) {
    ASTExpr2TypeExpr e2te(ctx);
    return e2te.transform<TypeExpression>(node, true);
}

}

}
