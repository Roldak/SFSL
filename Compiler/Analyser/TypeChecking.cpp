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

namespace sfsl {

namespace ast {

#define SAVE_SCOPE  sym::Scope* last = _curScope;
#define RESTORE_SCOPE _curScope = last;

// TYPE ASSIGNATION

TypeAssignation::TypeAssignation(std::shared_ptr<common::CompilationContext>& ctx) : ASTVisitor(ctx), _curScope(nullptr) {

}

void TypeAssignation::visit(ASTNode*) {

}

void TypeAssignation::visit(ModuleDecl* mod) {
    SAVE_SCOPE
    _curScope = mod->getSymbol()->getScope();

    ASTVisitor::visit(mod);

    RESTORE_SCOPE
}

void TypeAssignation::visit(ClassDecl *clss) {
    SAVE_SCOPE
    _curScope = clss->getSymbol()->getScope();

    ASTVisitor::visit(clss);

    RESTORE_SCOPE
}

void TypeAssignation::visit(DefineDecl* decl) {
    SAVE_SCOPE
    _curScope = decl->getSymbol()->getScope();

    ASTVisitor::visit(decl);

    RESTORE_SCOPE
}


void TypeAssignation::visit(FunctionCreation* func) {
    SAVE_SCOPE
    _curScope = func->getScope();

    ASTVisitor::visit(func);

    RESTORE_SCOPE
}

void TypeAssignation::visit(BinaryExpression* bin) {
    ASTVisitor::visit(this);

    bin->setType(bin->getLhs()->type()); // TODO : change that
}

void TypeAssignation::visit(TypeSpecifier* tps) {
    tps->getSpecified()->onVisit(this);

    if (type::Type* tpe = createType(tps->getTypeNode(), _ctx)) {
        Identifier* id = tps->getSpecified();
        type::Typed* tped = nullptr;

        if (id->getSymbol()->getSymbolType() == sym::SYM_VAR) {
            tped = static_cast<sym::VariableSymbol*>(id->getSymbol());
        }
        // TODO : if symbol type is SYM_DEF

        tped->setType(tpe);
    } else {
        _ctx.get()->reporter().error(*tps->getTypeNode(), "Expression is not a type");
    }
}

void TypeAssignation::visit(Block* block) {
    SAVE_SCOPE
    _curScope = block->getScope();

    ASTVisitor::visit(block);

    RESTORE_SCOPE
}

void TypeAssignation::visit(IfExpression *ifexpr) {
    ASTVisitor::visit(this);
}

void TypeAssignation::visit(IntLitteral* intlit) {

}

void TypeAssignation::visit(RealLitteral* reallit) {

}

// TYPE CHECK

TypeCheck::TypeCheck(std::shared_ptr<common::CompilationContext> &ctx) : ASTVisitor(ctx) {

}

void TypeCheck::visit(Statement* stmt) {

}

void TypeCheck::visit(MemberAccess* mac) {

}


}

}
