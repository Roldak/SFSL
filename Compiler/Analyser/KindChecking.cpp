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
        tdecl->getSymbol()->setKind(tdecl->getExpression()->kind());
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
    ASTVisitor::visit(tcall);

    if (kind::TypeConstructorKind* k = kind::getIf<kind::TypeConstructorKind>(tcall->getCallee()->kind())) {

        const std::vector<TypeExpression*> callArgs = tcall->getArgs();
        const std::vector<kind::Kind*> argkinds = k->getArgKinds();

        if (callArgs.size() != argkinds.size()) {
            _rep.error(*tcall, "Wrong number of type arguments. Expected " +
                       utils::T_toString(argkinds.size()) + ", found " + utils::T_toString(callArgs.size()));
            tcall->setKind(kind::Kind::NotYetDefined());
            return;
        }

        for (size_t i = 0; i < callArgs.size(); ++i) {
            if (!callArgs[i]->kind()->isSubKindOf(argkinds[i])) {
                _rep.error(*callArgs[i], "Kind mismatch. Expected " + argkinds[i]->toString() +
                           ", found " + callArgs[i]->kind()->toString());
                tcall->setKind(kind::Kind::NotYetDefined());
                return;
            }
        }

        tcall->setKind(k->getRetKind());

    } else {
        _rep.error(*tcall, "Kind mismatch. Expected type constructor, found " + tcall->getCallee()->kind()->toString());
    }
}

void KindChecking::visit(TypeIdentifier* tident) {
    if (sym::Symbol* s = tident->getSymbol()) {
        tident->setKind(tryGetKindOfSymbol(s));
    }
}

void KindChecking::visit(TypeSpecifier* ts) {
    ASTVisitor::visit(ts);

    if (ts->getTypeNode()->kind()->getKindGenre() != kind::TYPE_KIND) {
        _rep.error(*ts->getTypeNode(), "Kind mismatch. Expected proper type, found " + ts->getTypeNode()->kind()->toString());
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
    }

    return nullptr;
}

}

}
