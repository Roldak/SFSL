//
//  CodeGenerator.cpp
//  SFSL
//
//  Created by Romain Beguet on 22.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "BytecodeGenerator.h"
#include "../Frontend/AST/Visitors/ASTTypeIdentifier.h"
#include "../Frontend/AST/Visitors/ASTSymbolExtractor.h"

#define START_WRITING_TO_CONSTANT_POOL \
    out::Cursor* __old = Here(); \
    Seek(*_constantPoolCursor);

#define RESTART_WRITING_TO_CONSTANT_POOL \
    __old = Here(); \
    Seek(*_constantPoolCursor);

#define STOP_WRITING_TO_CONSTANT_POOL \
    *_constantPoolCursor = Here(); \
    Seek(__old);

namespace sfsl {

namespace bc {

// BYTECODE GENERATOR

BytecodeGenerator::BytecodeGenerator(CompCtx_Ptr& ctx, out::CodeGenOutput<BCInstruction*>& out)
    : CodeGenerator(ctx, out), _constantPoolCursor(std::make_shared<out::Cursor*>(Here())) {

}

BytecodeGenerator::~BytecodeGenerator() {

}

BytecodeGenerator::BytecodeGenerator(CompCtx_Ptr& ctx, out::CodeGenOutput<BCInstruction*>& out, std::shared_ptr<out::Cursor*> constantPoolCursor)
    : CodeGenerator(ctx, out), _constantPoolCursor(constantPoolCursor) {

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

size_t BytecodeGenerator::getClassLoc(ClassDecl* clss) {
    return clss->getUserdata<ClassUserData>()->getClassLoc();
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

DefaultBytecodeGenerator::DefaultBytecodeGenerator(CompCtx_Ptr& ctx, out::CodeGenOutput<BCInstruction*>& out)
    :   BytecodeGenerator(ctx, out) {

}

DefaultBytecodeGenerator::DefaultBytecodeGenerator(CompCtx_Ptr& ctx, out::CodeGenOutput<BCInstruction*>& out, std::shared_ptr<out::Cursor*> constantPoolCursor)
    :   BytecodeGenerator(ctx, out, constantPoolCursor) {

}

DefaultBytecodeGenerator::~DefaultBytecodeGenerator() {

}

void DefaultBytecodeGenerator::visit(ASTNode*) {

}

void DefaultBytecodeGenerator::visit(Program* prog) {
    ASTImplicitVisitor::visit(prog);
}

void DefaultBytecodeGenerator::visit(ModuleDecl* module) {
    ASTImplicitVisitor::visit(module);
}

void DefaultBytecodeGenerator::visit(TypeDecl* tdecl) {
    ASTImplicitVisitor::visit(tdecl);
}

void DefaultBytecodeGenerator::visit(ClassDecl* clss){
    if (TRY_INSERT(_visitedClasses, clss)) {
        for (DefineDecl* def : clss->getDefs()) {
            def->onVisit(this);
        }

        START_WRITING_TO_CONSTANT_POOL

        ClassUserData* clssData = clss->getUserdata<ClassUserData>();

        for (sym::DefinitionSymbol* def : clssData->getDefs()) {
            Emit<LoadConst>(*clss, getDefLoc(def));
        }

        Emit<MakeClass>(*clss, clssData->getAttrCount(), clssData->getDefCount());
        Emit<StoreConst>(*clss, getClassLoc(clss));

        STOP_WRITING_TO_CONSTANT_POOL
    }
}

void DefaultBytecodeGenerator::visit(DefineDecl* decl) {
    if (decl->isExtern()) {
        return;
    }

    START_WRITING_TO_CONSTANT_POOL

    decl->getValue()->onVisit(this);
    Emit<StoreConst>(*decl, getDefLoc(decl->getSymbol()));

    STOP_WRITING_TO_CONSTANT_POOL
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
    ASTImplicitVisitor::visit(exp);
}

void DefaultBytecodeGenerator::visit(AssignmentExpression* aex) {
    aex->getRhs()->onVisit(this);
    AssignmentBytecodeGenerator abg(_ctx, _out, _constantPoolCursor);
    aex->getLhs()->onVisit(&abg);
}

void DefaultBytecodeGenerator::visit(TypeSpecifier* tps) {
    tps->getSpecified()->onVisit(this);
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
    } else {
        Emit<PushConstUnit>(*ifexpr->getThen());
    }

    // label for the end of the if
    BindLabel(outLabel);
}

void DefaultBytecodeGenerator::visit(MemberAccess* dot) {
    dot->getAccessed()->onVisit(this);

    if (dot->getSymbol()->getSymbolType() == sym::SYM_VAR) {
        Emit<LoadField>(*dot, getVarLoc(static_cast<sym::VariableSymbol*>(dot->getSymbol())));
    }
}

void DefaultBytecodeGenerator::visit(Tuple* tuple) {
    ASTImplicitVisitor::visit(tuple);

    if (tuple->getExpressions().size() == 0) {
        Emit<PushConstUnit>(*tuple);
    }
}

void DefaultBytecodeGenerator::visit(FunctionCreation* func) {
    if (type::ProperType* pt = type::getIf<type::ProperType>(func->type())) {
        pt->getClass()->onVisit(this);

        // TODO: Generate code to instantiate the class
    } else {
        Label* funcEnd = MakeLabel(*func, "mthd_end");
        Emit<MakeMethod>(*func, func->getUserdata<FuncUserData>()->getVarCount(), funcEnd);

        func->getBody()->onVisit(this);
        Emit<Return>(*func);

        BindLabel(funcEnd);
    }
}

void DefaultBytecodeGenerator::visit(FunctionCall* call) {
    if (call->getCallee()->type()->getTypeKind() == type::TYPE_METHOD) {
        if (isNodeOfType<MemberAccess>(call->getCallee(), _ctx)) {
            MemberAccess* dot = static_cast<MemberAccess*>(call->getCallee());
            dot->getAccessed()->onVisit(this);
        }
        else if (isNodeOfType<Identifier>(call->getCallee(), _ctx)) { // implicit this
            Emit<LoadStack>(*call->getCallee(), 0);
        }

        sym::DefinitionSymbol* def = static_cast<sym::DefinitionSymbol*>(ASTSymbolExtractor::extractSymbol(call->getCallee(), _ctx));
        size_t virtualLoc = def->getUserdata<VirtualDefUserData>()->getVirtualLocation();

        for (Expression* expr : call->getArgs()) {
            expr->onVisit(this);
        }

        Emit<VCall>(*call, virtualLoc, call->getArgs().size());
    }
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

void DefaultBytecodeGenerator::visit(This* ths) {
    Emit<LoadStack>(*ths, 0);
}

void DefaultBytecodeGenerator::visit(BoolLitteral* boollit) {
    Emit<PushConstBool>(*boollit, boollit->getValue());
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

DefaultBytecodeGenerator::AssignmentBytecodeGenerator::AssignmentBytecodeGenerator(CompCtx_Ptr& ctx, out::CodeGenOutput<BCInstruction*>& out, std::shared_ptr<out::Cursor*> constantPoolCursor)
    : BytecodeGenerator(ctx, out, constantPoolCursor) {

}

DefaultBytecodeGenerator::AssignmentBytecodeGenerator::~AssignmentBytecodeGenerator() {

}

void DefaultBytecodeGenerator::AssignmentBytecodeGenerator::visit(ASTNode*) {

}

void DefaultBytecodeGenerator::AssignmentBytecodeGenerator::visit(TypeSpecifier* tps) {
    tps->getSpecified()->onVisit(this);
}

void DefaultBytecodeGenerator::AssignmentBytecodeGenerator::visit(MemberAccess* dot) {
    DefaultBytecodeGenerator dbg(_ctx, _out, _constantPoolCursor);
    dot->getAccessed()->onVisit(&dbg);

    if (dot->getSymbol()->getSymbolType() == sym::SYM_VAR) {
        Emit<StoreField>(*dot, getVarLoc(static_cast<sym::VariableSymbol*>(dot->getSymbol())));
    }
}

void DefaultBytecodeGenerator::AssignmentBytecodeGenerator::visit(IfExpression* ifexpr) {
    Label* elseLabel = MakeLabel(*ifexpr, "else");
    Label* outLabel = MakeLabel(*ifexpr, "out");

    // if cond is false, jump to the else label
    DefaultBytecodeGenerator dbg(_ctx, _out, _constantPoolCursor);
    ifexpr->getCondition()->onVisit(&dbg);
    Emit<IfFalse>(*ifexpr->getCondition(), elseLabel);

    // code for the then part, plus the jump to the end of the if
    ifexpr->getThen()->onVisit(this);
    Emit<Jump>(*ifexpr->getThen(), outLabel);

    // label and code for the else part
    BindLabel(elseLabel);
    ifexpr->getElse()->onVisit(this); // no need to check if it has an else part because it is a requirement

    // label for the end of the if
    BindLabel(outLabel);
}

void DefaultBytecodeGenerator::AssignmentBytecodeGenerator::visit(Identifier* ident) {
    sym::VariableSymbol* var = static_cast<sym::VariableSymbol*>(ident->getSymbol());
    Emit<StoreStack>(*ident, getVarLoc(var));
}

}

}
