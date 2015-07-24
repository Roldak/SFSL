//
//  CodeGenerator.cpp
//  SFSL
//
//  Created by Romain Beguet on 22.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "BytecodeGenerator.h"

namespace sfsl {

namespace bc {

// BYTECODE GENERATOR

BytecodeGenerator::BytecodeGenerator(CompCtx_Ptr& ctx, out::CodeGenOutput<BCInstruction*>& out)
    : CodeGenerator(ctx, out) {

}

BytecodeGenerator::~BytecodeGenerator() {

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

Label* BytecodeGenerator::MakeLabel(const common::Positionnable& pos, const std::string& name) {
    Label* label = _mngr.New<Label>(name);
    label->setPos(pos);
    return label;
}

void BytecodeGenerator::BindLabel(Label* label) {
    Emit(label);
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

size_t BytecodeGenerator::getDefLoc(sym::DefinitionSymbol* def) {
    return def->getUserdata<DefUserData>()->getDefLoc();
}

size_t BytecodeGenerator::getVarLoc(sym::VariableSymbol* var) {
    return var->getUserdata<VarUserData>()->getVarLoc();
}

// DEFAULT BYTECODE GENERATOR

DefaultBytecodeGenerator::DefaultBytecodeGenerator(CompCtx_Ptr &ctx, out::CodeGenOutput<BCInstruction*> &out)
    :   BytecodeGenerator(ctx, out) {

}

DefaultBytecodeGenerator::~DefaultBytecodeGenerator() {

}

void DefaultBytecodeGenerator::visit(ASTNode*) {

}

void DefaultBytecodeGenerator::visit(Program* prog) {
    ASTVisitor::visit(prog);
}

void DefaultBytecodeGenerator::visit(ModuleDecl* module) {
    ASTVisitor::visit(module);
}

void DefaultBytecodeGenerator::visit(TypeDecl* tdecl) {

}

void DefaultBytecodeGenerator::visit(ClassDecl* clss){

}

void DefaultBytecodeGenerator::visit(DefineDecl* decl) {
    decl->getValue()->onVisit(this);
    Emit<StoreConst>(*decl, getDefLoc(decl->getSymbol()));
}

void DefaultBytecodeGenerator::visit(ProperTypeKindSpecifier* ptks) {

}

void DefaultBytecodeGenerator::visit(TypeConstructorKindSpecifier* tcks) {

}

void DefaultBytecodeGenerator::visit(TypeMemberAccess* tdot) {

}

void DefaultBytecodeGenerator::visit(TypeTuple* ttuple) {

}

void DefaultBytecodeGenerator::visit(TypeConstructorCreation* typeconstructor) {

}

void DefaultBytecodeGenerator::visit(TypeConstructorCall* tcall) {

}

void DefaultBytecodeGenerator::visit(TypeIdentifier* tident) {

}

void DefaultBytecodeGenerator::visit(KindSpecifier* ks) {

}

void DefaultBytecodeGenerator::visit(ExpressionStatement* exp) {
    ASTVisitor::visit(exp);
}

void DefaultBytecodeGenerator::visit(BinaryExpression* bin) {
    ASTVisitor::visit(bin);
}

void DefaultBytecodeGenerator::visit(AssignmentExpression* aex) {
    aex->getRhs()->onVisit(this);
    AssignmentBytecodeGenerator abg(_ctx, _out);
    aex->getLhs()->onVisit(&abg);
}

void DefaultBytecodeGenerator::visit(TypeSpecifier* tps) {
    Emit<PushConstUnit>(*tps);
}

void DefaultBytecodeGenerator::visit(Block* block) {
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

void DefaultBytecodeGenerator::visit(IfExpression* ifexpr) {
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

    if (ifexpr->getElse()) {
        ifexpr->getElse()->onVisit(this);
    }

    // label for the end of the if
    BindLabel(outLabel);
}

void DefaultBytecodeGenerator::visit(MemberAccess* dot) {

}

void DefaultBytecodeGenerator::visit(Tuple* tuple) {
    ASTVisitor::visit(tuple);
}

void DefaultBytecodeGenerator::visit(FunctionCreation* func) {
    Label* funcEnd = MakeLabel(*func, "func_end");
    Emit<MakeFunction>(*func, func->getUserdata<FuncUserData>()->getVarCount(), funcEnd);

    func->getBody()->onVisit(this);
    Emit<Return>(*func);

    BindLabel(funcEnd);
}

void DefaultBytecodeGenerator::visit(FunctionCall* call) {
    ASTVisitor::visit(call);
}

void DefaultBytecodeGenerator::visit(Identifier* ident) {
    switch (ident->getSymbol()->getSymbolType()) {
    case sym::SYM_VAR: {
        sym::VariableSymbol* var = static_cast<sym::VariableSymbol*>(ident->getSymbol());
        Emit<LoadStack>(*ident, getVarLoc(var));
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

void DefaultBytecodeGenerator::visit(IntLitteral* intlit) {
    Emit<PushConstInt>(*intlit, intlit->getValue());
}

void DefaultBytecodeGenerator::visit(RealLitteral* reallit) {
    Emit<PushConstReal>(*reallit, reallit->getValue());
}

// ASSIGNMENT BYTECODE GENERATOR

DefaultBytecodeGenerator::AssignmentBytecodeGenerator::AssignmentBytecodeGenerator(CompCtx_Ptr& ctx, out::CodeGenOutput<BCInstruction*>& out)
    : BytecodeGenerator(ctx, out) {

}

DefaultBytecodeGenerator::AssignmentBytecodeGenerator::~AssignmentBytecodeGenerator() {

}

void DefaultBytecodeGenerator::AssignmentBytecodeGenerator::visit(ASTNode*) {

}

void DefaultBytecodeGenerator::AssignmentBytecodeGenerator::visit(Identifier* ident) {
    sym::VariableSymbol* var = static_cast<sym::VariableSymbol*>(ident->getSymbol());
    Emit<StoreStack>(*ident, getVarLoc(var));
}

}

}
