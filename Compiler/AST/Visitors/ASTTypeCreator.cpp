//
//  ASTTypeCreator.cpp
//  SFSL
//
//  Created by Romain Beguet on 31.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "ASTTypeCreator.h"

namespace sfsl {

namespace ast {

ASTTypeCreator::ASTTypeCreator(CompCtx_Ptr& ctx) : ASTVisitor(ctx), _created(nullptr) {

}

ASTTypeCreator::~ASTTypeCreator() {

}

void ASTTypeCreator::visit(ASTNode* node) {
    // do not throw an exception
}

void ASTTypeCreator::visit(ClassDecl *clss) {
    _created = _mngr.New<type::ObjectType>(clss);
}

void ASTTypeCreator::visit(MemberAccess* mac) {
    createTypeFromSymbolic(mac, *mac);
}

void ASTTypeCreator::visit(Identifier* ident) {
    createTypeFromSymbolic(ident, *ident);
}

type::Type* ASTTypeCreator::getCreatedType() const {
    return _created;
}

void ASTTypeCreator::createTypeFromSymbolic(sym::Symbolic<sym::Symbol> *symbolic, common::Positionnable& pos) {
    if (sym::Symbol* symbol = symbolic->getSymbol()) {
        if (symbol->getSymbolType() != sym::SYM_TPE) {
            _ctx.get()->reporter().error(pos, "Symbol does not refer a class");
            return;
        }

        sym::TypeSymbol* ts = static_cast<sym::TypeSymbol>(symbol);

        if (ts->type() == type::Type::NotYetDefined()) {
            _created = createType(ts->getTypeDecl()->getExpression(), _ctx);
            ts->setType(_created);
        } else {
            _created = ts->type();
        }
    }
}

}

}
