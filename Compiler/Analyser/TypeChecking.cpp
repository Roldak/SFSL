//
//  TypeChecking.cpp
//  SFSL
//
//  Created by Romain Beguet on 30.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "TypeChecking.h"
#include "../AST/Visitors/ASTTypeIdentifier.h"
#include "../AST/Visitors/ASTTypeCreator.h"
#include "../AST/Symbols/Scope.h"

namespace sfsl {

namespace ast {

#define SAVE_SCOPE  sym::Scope* last = _curScope;
#define RESTORE_SCOPE _curScope = last;

// TYPE CHECK

TypeCheking::TypeCheking(CompCtx_Ptr& ctx, const sym::SymbolResolver& res)
    : ASTVisitor(ctx), _curScope(nullptr), _res(res), _rep(ctx.get()->reporter()) {

}

void TypeCheking::visit(ASTNode*) {

}

void TypeCheking::visit(ModuleDecl* mod) {
    SAVE_SCOPE
    _curScope = mod->getSymbol()->getScope();

    ASTVisitor::visit(mod);

    RESTORE_SCOPE
}

void TypeCheking::visit(ClassDecl* clss) {
    SAVE_SCOPE
    _curScope = clss->getSymbol()->getScope();

    ASTVisitor::visit(clss);

    RESTORE_SCOPE
}

void TypeCheking::visit(DefineDecl* decl) {
    SAVE_SCOPE
    _curScope = decl->getSymbol()->getScope();

    ASTVisitor::visit(decl);

    decl->getName()->setType(decl->getValue()->type());
    static_cast<sym::DefinitionSymbol*>(decl->getSymbol())->setType(decl->getValue()->type());

    RESTORE_SCOPE
}

void TypeCheking::visit(ExpressionStatement* exp) {
    ASTVisitor::visit(exp);
    exp->setType(exp->getExpression()->type());
}

void TypeCheking::visit(BinaryExpression* bin) {
    ASTVisitor::visit(bin);

    bin->setType(bin->getLhs()->type()); // TODO : make it right
}

void TypeCheking::visit(TypeSpecifier* tps) {
    tps->getSpecified()->onVisit(this);

    if (type::Type* tpe = createType(tps->getTypeNode(), _ctx)) {
        Identifier* id = tps->getSpecified();
        type::Typed* tped = nullptr;

        if (id->getSymbol()->getSymbolType() == sym::SYM_VAR) {
            tped = static_cast<sym::VariableSymbol*>(id->getSymbol());
        } else if (id->getSymbol()->getSymbolType() == sym::SYM_DEF) {
            tped = static_cast<sym::DefinitionSymbol*>(id->getSymbol());
        }

        tped->setType(tpe);
    } else {
        _ctx.get()->reporter().error(*tps->getTypeNode(), "Expression is not a type");
    }
}

void TypeCheking::visit(Block* block) {
    SAVE_SCOPE
    _curScope = block->getScope();

    ASTVisitor::visit(block);

    const std::vector<Expression*>& stats = block->getStatements();
    if (stats.size() > 0) {
        block->setType(stats.back()->type());
    } else {
        block->setType(_res.Unit());
    }

    RESTORE_SCOPE
}

void TypeCheking::visit(IfExpression* ifexpr) {
    ASTVisitor::visit(ifexpr);
    //ifexpr->setType(ifexpr->getThen());
}

void TypeCheking::visit(MemberAccess* dot) {

}

void TypeCheking::visit(Tuple* tuple) {

}

void TypeCheking::visit(FunctionCreation* func) {
    SAVE_SCOPE
    _curScope = func->getScope();

    ASTVisitor::visit(func);

    RESTORE_SCOPE
}

void TypeCheking::visit(FunctionCall* call) {
    ASTVisitor::visit(call);
}

void TypeCheking::visit(Identifier* ident) {
    if (sym::Symbol* sym = ident->getSymbol()) {
        if (sym->getSymbolType() == sym::SYM_VAR) {
            ident->setType(static_cast<sym::VariableSymbol*>(ident->getSymbol())->type());
        }
    }
}

void TypeCheking::visit(IntLitteral* intlit) {
    intlit->setType(_res.Int());
}

void TypeCheking::visit(RealLitteral* reallit) {
    reallit->setType(_res.Real());
}

}

}
