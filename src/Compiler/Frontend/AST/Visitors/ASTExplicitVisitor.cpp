//
//  ASTExplicitVisitor.cpp
//  SFSL
//
//  Created by Romain Beguet on 26.08.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "ASTExplicitVisitor.h"

namespace sfsl {

namespace ast {

ASTExplicitVisitor::ASTExplicitVisitor(CompCtx_Ptr& ctx) : ASTVisitor(ctx) {

}

ASTExplicitVisitor::~ASTExplicitVisitor() {

}

void ASTExplicitVisitor::visit(ASTNode*) {
    // do not throw an exception
}

void ASTExplicitVisitor::visit(Program*) {

}

void ASTExplicitVisitor::visit(ModuleDecl*) {

}

void ASTExplicitVisitor::visit(TypeDecl*) {

}

void ASTExplicitVisitor::visit(ClassDecl*){

}

void ASTExplicitVisitor::visit(DefineDecl*) {

}

void ASTExplicitVisitor::visit(ProperTypeKindSpecifier*) {

}

void ASTExplicitVisitor::visit(TypeConstructorKindSpecifier*) {

}

void ASTExplicitVisitor::visit(FunctionTypeDecl*) {

}

void ASTExplicitVisitor::visit(TypeMemberAccess*) {

}

void ASTExplicitVisitor::visit(TypeTuple*) {

}

void ASTExplicitVisitor::visit(TypeConstructorCreation*) {

}

void ASTExplicitVisitor::visit(TypeConstructorCall*) {

}

void ASTExplicitVisitor::visit(TypeIdentifier*) {

}

void ASTExplicitVisitor::visit(TypeToBeInferred*) {

}

void ASTExplicitVisitor::visit(TypeParameter*) {

}

void ASTExplicitVisitor::visit(ExpressionStatement*) {

}

void ASTExplicitVisitor::visit(AssignmentExpression*) {

}

void ASTExplicitVisitor::visit(TypeSpecifier*) {

}

void ASTExplicitVisitor::visit(Block*) {

}

void ASTExplicitVisitor::visit(IfExpression*) {

}

void ASTExplicitVisitor::visit(MemberAccess*) {

}

void ASTExplicitVisitor::visit(Tuple*) {

}

void ASTExplicitVisitor::visit(FunctionCreation*) {

}

void ASTExplicitVisitor::visit(FunctionCall*) {

}

void ASTExplicitVisitor::visit(Instantiation*) {

}

void ASTExplicitVisitor::visit(Identifier*) {

}

void ASTExplicitVisitor::visit(This*) {

}

void ASTExplicitVisitor::visit(BoolLitteral*) {

}

void ASTExplicitVisitor::visit(IntLitteral*) {

}

void ASTExplicitVisitor::visit(RealLitteral*) {

}

void ASTExplicitVisitor::visit(StringLitteral*) {

}

}

}
