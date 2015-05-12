//
//  TypeChecking.cpp
//  SFSL
//
//  Created by Romain Beguet on 30.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "KindChecking.h"
#include "../AST/Visitors/ASTTypeIdentifier.h"
#include "../AST/Visitors/ASTTypeCreator.h"
#include "../AST/Symbols/Scope.h"

namespace sfsl {

namespace ast {

// TYPE CHECK

KindChecking::KindChecking(CompCtx_Ptr& ctx)
    : ASTVisitor(ctx), _rep(ctx->reporter()) {

}

KindChecking::~KindChecking() {

}

void KindChecking::visit(ASTNode*) {

}

void KindChecking::visit(TypeDecl* tdecl) {
    if (_visitedTypeDefs.find(tdecl) == _visitedTypeDefs.end()) {
        _visitedTypeDefs.emplace(tdecl);

        tdecl->getExpression()->onVisit(this);

        // kind inference
        tdecl->getName()->setKind(tdecl->getExpression()->kind());
        static_cast<sym::TypeSymbol*>(tdecl->getSymbol())->setKind(tdecl->getExpression()->kind());
    }
}

void KindChecking::visit(ClassDecl* clss) {
    ASTVisitor::visit(clss);

    clss->setKind(kind::TypeKind::create());
}

void KindChecking::visit(TypeMemberAccess* tdot) {
    tdot->getAccessed()->onVisit(this);

    if (sym::Symbol* s = tdot->getSymbol()) {
        tdot->setKind(tryGetKindOfSymbol(s));
    }
}

void KindChecking::visit(TypeTuple* ttuple) {
    ASTVisitor::visit(ttuple);
}

void KindChecking::visit(TypeConstructorCreation* tc) {
    ASTVisitor::visit(tc);

    std::vector<kind::Kind*> argKinds;
    kind::Kind* retKind;

    if (isNodeOfType<TypeTuple>(tc->getArgs(), _ctx)) {
        const std::vector<TypeExpression*>& exprs = static_cast<TypeTuple*>(tc->getArgs())->getExpressions();
        argKinds.resize(exprs.size());

        for (size_t i = 0; i < exprs.size(); ++i) {
            argKinds[i] = exprs[i]->kind();
        }
    } else {
        // TODO : the rest
    }

    retKind = tc->getBody()->kind();

    tc->setKind(_mngr.New<kind::TypeConstructorKind>(argKinds, retKind));

    _rep.info(*tc->getArgs(), tc->kind()->toString());
}

void KindChecking::visit(TypeConstructorCall* tcall) {

}

void KindChecking::visit(TypeIdentifier* tident) {
    if (sym::Symbol* s = tident->getSymbol()) {
        tident->setKind(tryGetKindOfSymbol(s));
    }
}

kind::Kind* KindChecking::tryGetKindOfSymbol(sym::Symbol* sym) {
    if (sym->getSymbolType() == sym::SYM_TPE) {
        sym::TypeSymbol* tpesym = static_cast<sym::TypeSymbol*>(sym);
        tpesym->getTypeDecl()->onVisit(this);

        if (tpesym->kind() == kind::Kind::NotYetDefined()) {
            _rep.error(*sym, "Kind of " + sym->getName() + " cannot be inferred because of a cyclic dependency");
        }

        return tpesym->kind();
    } else {
        _rep.error(*sym, "Symbol '" + sym->getName() + "' is not kinded");
    }

    return nullptr;
}

}

}