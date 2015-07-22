//
//  CodeGenerator.cpp
//  SFSL
//
//  Created by Romain Beguet on 21.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "BytecodeGenerator.h"

namespace sfsl {

namespace bc {

// BYTE CODE GENERATOR

BytecodeGenerator::BytecodeGenerator(CompCtx_Ptr &ctx, out::CodeGenOutput<BCInstruction> &out) : out::CodeGenerator<BCInstruction>(ctx, out) {

}

BytecodeGenerator::~BytecodeGenerator() {

}

void BytecodeGenerator::visit(ASTNode*) {

}

void BytecodeGenerator::visit(Program* prog) {

}

void BytecodeGenerator::visit(ModuleDecl* module) {

}

void BytecodeGenerator::visit(TypeDecl* tdecl) {

}

void BytecodeGenerator::visit(ClassDecl* clss){

}

void BytecodeGenerator::visit(DefineDecl* decl) {

}

void BytecodeGenerator::visit(ProperTypeKindSpecifier* ptks) {

}

void BytecodeGenerator::visit(TypeConstructorKindSpecifier* tcks) {

}

void BytecodeGenerator::visit(TypeMemberAccess* tdot) {

}

void BytecodeGenerator::visit(TypeTuple* ttuple) {

}

void BytecodeGenerator::visit(TypeConstructorCreation* typeconstructor) {

}

void BytecodeGenerator::visit(TypeConstructorCall* tcall) {

}

void BytecodeGenerator::visit(TypeIdentifier* tident) {

}

void BytecodeGenerator::visit(KindSpecifier* ks) {

}

void BytecodeGenerator::visit(ExpressionStatement* exp) {

}

void BytecodeGenerator::visit(BinaryExpression* bin) {

}

void BytecodeGenerator::visit(AssignmentExpression* aex) {

}

void BytecodeGenerator::visit(TypeSpecifier* tps) {

}

void BytecodeGenerator::visit(Block* block) {

}

void BytecodeGenerator::visit(IfExpression* ifexpr) {

}

void BytecodeGenerator::visit(MemberAccess* dot) {

}

void BytecodeGenerator::visit(Tuple* tuple) {

}

void BytecodeGenerator::visit(FunctionCreation* func) {

}

void BytecodeGenerator::visit(FunctionCall* call) {

}

void BytecodeGenerator::visit(Identifier* ident) {

}

void BytecodeGenerator::visit(IntLitteral* intlit) {

}

void BytecodeGenerator::visit(RealLitteral* reallit) {

}

}

}
