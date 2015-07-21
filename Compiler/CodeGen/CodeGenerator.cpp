//
//  CodeGenerator.cpp
//  SFSL
//
//  Created by Romain Beguet on 21.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "CodeGenerator.h"
#include "../AST/Visitors/ASTTypeIdentifier.h"
#include "../AST/Visitors/ASTTypeCreator.h"
#include "../AST/Symbols/Scope.h"

namespace sfsl {

namespace out {

// TYPE CHECK

CodeGenerator::CodeGenerator(CompCtx_Ptr& ctx, CodeGenOutput& out)
    : ASTVisitor(ctx), _out(out) {

}

CodeGenerator::~CodeGenerator() {

}

void CodeGenerator::visit(ASTNode*) {

}

void CodeGenerator::visit(Program* prog) {

}

void CodeGenerator::visit(ModuleDecl* module) {

}

void CodeGenerator::visit(TypeDecl* tdecl) {

}

void CodeGenerator::visit(DefineDecl* decl) {

}

void CodeGenerator::visit(ProperTypeKindSpecifier* ptks) {

}

void CodeGenerator::visit(TypeConstructorKindSpecifier* tcks) {

}

void CodeGenerator::visit(TypeMemberAccess* tdot) {

}

void CodeGenerator::visit(TypeTuple* ttuple) {

}

void CodeGenerator::visit(TypeConstructorCreation* typeconstructor) {

}

void CodeGenerator::visit(TypeConstructorCall* tcall) {

}

void CodeGenerator::visit(TypeIdentifier* tident) {

}

void CodeGenerator::visit(KindSpecifier* ks) {

}

void CodeGenerator::visit(ExpressionStatement* exp) {

}

void CodeGenerator::visit(BinaryExpression* bin) {

}

void CodeGenerator::visit(AssignmentExpression* aex) {

}

void CodeGenerator::visit(TypeSpecifier* tps) {

}

void CodeGenerator::visit(Block* block) {

}

void CodeGenerator::visit(IfExpression* ifexpr) {

}

void CodeGenerator::visit(MemberAccess* dot) {

}

void CodeGenerator::visit(Tuple* tuple) {

}

void CodeGenerator::visit(FunctionCreation* func) {

}

void CodeGenerator::visit(FunctionCall* call) {

}

void CodeGenerator::visit(Identifier* ident) {

}

void CodeGenerator::visit(IntLitteral* intlit) {

}

void CodeGenerator::visit(RealLitteral* reallit) {

}

}

}
