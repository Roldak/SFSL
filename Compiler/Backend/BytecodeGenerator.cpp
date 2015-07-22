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

namespace sfsl {

namespace bc {

// BYTE CODE GENERATOR

BytecodeGenerator::BytecodeGenerator(CompCtx_Ptr &ctx, out::CodeGenOutput<BCInstruction*> &out)
    :   out::CodeGenerator<BCInstruction*>(ctx, out),
        _currentConstCount(0), _currentVarCount(0) {

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
    decl->getValue()->onVisit(this);

    Emit<StoreConst>(*decl, getDefLoc(decl->getSymbol()));
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
    const std::vector<Expression*>& exprs(block->getStatements());

    if (exprs.size() > 0) {
        for (size_t i = 0; i < exprs.size() - 1; ++i) {
            exprs[i]->onVisit(this);
            Emit<Pop>(*exprs[i]);
        }
        exprs.back()->onVisit(this);
    } else {
        Emit<PushConstUnit>(*block);
    }
}

void BytecodeGenerator::visit(IfExpression* ifexpr) {
    Label* elseLabel = MakeLabel(*ifexpr, "else");
    Label* outLabel = MakeLabel(*ifexpr, "out");

    // if cond is false, jump to the else label
    ifexpr->getCondition()->onVisit(this);
    Emit<IfFalse>(*ifexpr->getCondition(), elseLabel);

    // code for the then part, plus the jump to the end of the if
    ifexpr->getThen()->onVisit(this);
    Emit<Jump>(*ifexpr->getThen(), outLabel);

    // label and code for the else part
    BindLabel(elseLabel);
    ifexpr->getElse()->onVisit(this);

    // label for the end of the if
    BindLabel(outLabel);
}

void BytecodeGenerator::visit(MemberAccess* dot) {

}

void BytecodeGenerator::visit(Tuple* tuple) {
    ASTVisitor::visit(tuple);
}

void BytecodeGenerator::visit(FunctionCreation* func) {
    SAVE_MEMBER(_currentVarCount);

    _currentVarCount = 0;

    out::Cursor* funcBegin = Here();

    ASTVisitor::visit(func);
    Emit<Return>(*func);
    Label* funcEnd = MakeLabel(*func, "func_end");
    BindLabel(funcEnd);

    Seek(funcBegin);
    Emit<MakeFunction>(*func, _currentVarCount, funcEnd);

    Seek(End());

    RESTORE_MEMBER(_currentVarCount);
}

void BytecodeGenerator::visit(FunctionCall* call) {

}

void BytecodeGenerator::visit(Identifier* ident) {
    switch (ident->getSymbol()->getSymbolType()) {
    case sym::SYM_VAR: {
        sym::VariableSymbol* var = static_cast<sym::VariableSymbol*>(ident->getSymbol());
        Emit<StackLoad>(*ident, getVarLoc(var));
        break;
    }

    case sym::SYM_DEF: {
        sym::DefinitionSymbol* def = static_cast<sym::DefinitionSymbol*>(ident->getSymbol());
        Emit<LoadConst>(*ident, getDefLoc(def));
        break;
    }

    default:
        break;
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

Label* BytecodeGenerator::MakeLabel(const common::Positionnable &pos, const std::string& name) {
    Label* label = _mngr.New<Label>(name);
    label->setPos(pos);
    return label;
}

void BytecodeGenerator::BindLabel(Label* label) {
    Emit(label);
}

size_t BytecodeGenerator::getVarLoc(sym::VariableSymbol* var) {
    return var->getUserdata<VarUserData>()->getVarLoc();
}

size_t BytecodeGenerator::getDefLoc(sym::DefinitionSymbol* def) {
    if (DefUserData* ddata = def->getUserdata<DefUserData>()) {
        return ddata->getDefLoc();
    } else {
        def->setUserdata(_mngr.New<DefUserData>(_currentConstCount++));
        return _currentConstCount - 1;
    }
}

template<typename T, typename... Args>
T* BytecodeGenerator::Emit(const common::Positionnable& pos, Args... args) {
    T* instr = _mngr.New<T>(std::forward<Args>(args)...);
    instr->setPos(pos);
    _out << instr;
    return instr;
}

template<typename T>
T* BytecodeGenerator::Emit(T* instr) {
    _out << instr;
    return instr;
}

// VARIABLE USER DATA

VarUserData::VarUserData(size_t loc) : _loc(loc) {

}

VarUserData::~VarUserData() {

}

size_t VarUserData::getVarLoc() const {
    return _loc;
}

DefUserData::DefUserData(size_t loc) : _loc(loc) {

}

DefUserData::~DefUserData() {

}

size_t DefUserData::getDefLoc() const {
    return _loc;
}

}

}
