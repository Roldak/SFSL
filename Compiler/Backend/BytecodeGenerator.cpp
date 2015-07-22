//
//  CodeGenerator.cpp
//  SFSL
//
//  Created by Romain Beguet on 21.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "BytecodeGenerator.h"

#define SAVE_MEMBER(memberName) auto __old##memberName = memberName
#define RESTORE_MEMBER(memberName) memberName = __old##memberName
#define CURSOR_HERE(varName) out::Cursor* varName = Here()

namespace sfsl {

namespace bc {

// BYTE CODE GENERATOR

BytecodeGenerator::BytecodeGenerator(CompCtx_Ptr &ctx, out::CodeGenOutput<BCInstruction*> &out)
    : out::CodeGenerator<BCInstruction*>(ctx, out) {

}

BytecodeGenerator::~BytecodeGenerator() {

}

void BytecodeGenerator::visit(ASTNode*) {

}

void BytecodeGenerator::visit(Program* prog) {
    ASTVisitor::visit(prog);
}

void BytecodeGenerator::visit(ModuleDecl* module) {
    ASTVisitor::visit(module);
}

void BytecodeGenerator::visit(TypeDecl* tdecl) {

}

void BytecodeGenerator::visit(ClassDecl* clss){

}

void BytecodeGenerator::visit(DefineDecl* decl) {
    ASTVisitor::visit(decl);
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
    ASTVisitor::visit(exp);
}

void BytecodeGenerator::visit(BinaryExpression* bin) {
    ASTVisitor::visit(bin);
}

void BytecodeGenerator::visit(AssignmentExpression* aex) {
    ASTVisitor::visit(aex);
}

void BytecodeGenerator::visit(TypeSpecifier* tps) {
    sym::VariableSymbol* var = static_cast<sym::VariableSymbol*>(tps->getSpecified()->getSymbol());
    var->setUserdata(_mngr.New<VarUserData>(_currentVarCount++));
}

void BytecodeGenerator::visit(Block* block) {
    ASTVisitor::visit(block);
}

void BytecodeGenerator::visit(IfExpression* ifexpr) {

}

void BytecodeGenerator::visit(MemberAccess* dot) {

}

void BytecodeGenerator::visit(Tuple* tuple) {
    ASTVisitor::visit(tuple);
}

void BytecodeGenerator::visit(FunctionCreation* func) {
    SAVE_MEMBER(_currentVarCount);

    _currentVarCount = 0;

    CURSOR_HERE(funcBegin);

    ASTVisitor::visit(func);

    Seek(funcBegin);
    Emit<MakeFunction>(*func, _currentVarCount);
    Seek(End());

    RESTORE_MEMBER(_currentVarCount);
}

void BytecodeGenerator::visit(FunctionCall* call) {

}

void BytecodeGenerator::visit(Identifier* ident) {
    if (ident->getSymbol()->getSymbolType() == sym::SYM_VAR) {
        sym::VariableSymbol* var = static_cast<sym::VariableSymbol*>(ident->getSymbol());
        Emit<StackLoad>(*ident, var->getUserdata<VarUserData>()->getVarLoc());
    }
}

void BytecodeGenerator::visit(IntLitteral* intlit) {
    Emit<PushConstInt>(*intlit, intlit->getValue());
}

void BytecodeGenerator::visit(RealLitteral* reallit) {
    Emit<PushConstReal>(*reallit, reallit->getValue());
}

out::Cursor* BytecodeGenerator::Here() const {
    return _out.here();
}

out::Cursor* BytecodeGenerator::End() const {
    return _out.end();
}

void BytecodeGenerator::Seek(out::Cursor* cursor) {
    _out.seek(cursor);
}

template<typename T, typename... Args>
void BytecodeGenerator::Emit(const common::Positionnable& pos, Args... args) {
    T* instr = _mngr.New<T>(std::forward<Args>(args)...);
    instr->setPos(pos);
    _out << instr;
}

// VARIABLE USER DATA

VarUserData::VarUserData(size_t loc) : _loc(loc) {

}

VarUserData::~VarUserData() {

}

size_t VarUserData::getVarLoc() const {
    return _loc;
}

}

}
