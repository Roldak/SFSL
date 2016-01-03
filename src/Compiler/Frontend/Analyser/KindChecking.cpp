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
#include "../AST/Visitors/ASTKindCreator.h"
#include "../AST/Visitors/ASTSymbolExtractor.h"

#include "../Symbols/Scope.h"

namespace sfsl {

namespace ast {

// TYPE CHECK

KindChecking::KindChecking(CompCtx_Ptr& ctx)
    : ASTImplicitVisitor(ctx), _rep(ctx->reporter()), _mustDefer(true), _insideMemberAccess(false) {

}

KindChecking::~KindChecking() {

}

void KindChecking::visit(ASTNode*) {

}

void KindChecking::visit(Program* prog) {
    ASTImplicitVisitor::visit(prog);
    visitDeferredExpressions();
}

void KindChecking::visit(TypeDecl* tdecl) {
    if (TRY_INSERT(_visitedTypeDefs, tdecl)) {
        tdecl->getExpression()->onVisit(this);

        // kind inference
        tdecl->getName()->setKind(tdecl->getExpression()->kind());
        tdecl->getSymbol()->setKind(tdecl->getExpression()->kind());
    }
}

void KindChecking::visit(ClassDecl* clss) {
    clss->setKind(kind::ProperKind::create());

    if (_mustDefer) {
        _deferredExpressions.emplace(clss);
    } else {
        _mustDefer = true;
        ASTImplicitVisitor::visit(clss);
        _mustDefer = false;

        if (TypeExpression* p = clss->getParent()) {
            if (p->kind()->getKindGenre() != kind::KIND_PROPER) {
                _rep.error(*p, "Kind mismatch. Expected proper type, found " + p->kind()->toString());
            }
        }
    }
}

void KindChecking::visit(DefineDecl* decl) {
    ASTImplicitVisitor::visit(decl);

    if (TypeExpression* expr = decl->getTypeSpecifier()) {
        if (expr->kind()->getKindGenre() != kind::KIND_PROPER) {
            _rep.error(*expr, "Kind mismatch. Expected proper type, found " + expr->kind()->toString());
        }
    }
}

void KindChecking::visit(FunctionTypeDecl* ftdecl) {
    ASTImplicitVisitor::visit(ftdecl);

    for (TypeExpression* texpr : ftdecl->getArgTypes()) {
        if (texpr->kind()->getKindGenre() != kind::KIND_PROPER) {
            _rep.error(*texpr, "Kind mismatch. Expected proper type, found " + texpr->kind()->toString());
        }
    }

    if (ftdecl->getRetType()->kind()->getKindGenre() != kind::KIND_PROPER) {
        _rep.error(*ftdecl->getRetType(), "Kind mismatch. Expected proper type, found " + ftdecl->getRetType()->kind()->toString());
    }

    ftdecl->setKind(kind::ProperKind::create());
}

void KindChecking::visit(TypeMemberAccess* tdot) {
    SAVE_MEMBER_AND_SET(_insideMemberAccess, true)

    tdot->getAccessed()->onVisit(this);

    RESTORE_MEMBER(_insideMemberAccess)

    if (sym::Symbol* s = tdot->getSymbol()) {
        if (kind::Kind* k = tryGetKindOfSymbol(s)) {
            tdot->setKind(k);
        } else if (!_insideMemberAccess) {
            _rep.error(*tdot, "Symbol " + s->getName() + " does not have any kind");
        }
    } else if (type::Type* t = ASTTypeCreator::createType(tdot->getAccessed(), _ctx)) {
        if (type::ProperType* pt = type::getIf<type::ProperType>(t->applyTCCallsOnly(_ctx))) {
            pt->getClass()->getScope()->assignSymbolic(*tdot, tdot->getMember()->getValue()); // update member access symbolic for potential later use
            tdot->setKind(kind::ProperKind::create());
        } else {
            _rep.error(*tdot->getAccessed(), "Type " + t->toString() + " cannot have any members");
        }
    }
}

void KindChecking::visit(TypeTuple* ttuple) {
    ASTImplicitVisitor::visit(ttuple);
}

void KindChecking::visit(TypeConstructorCreation* tc) {
    ASTImplicitVisitor::visit(tc);

    std::vector<TypeExpression*> exprs;

    if (isNodeOfType<TypeTuple>(tc->getArgs(), _ctx)) { // form is `[] => ...` or `[exp, exp] => ...`, ...
        exprs = static_cast<TypeTuple*>(tc->getArgs())->getExpressions();
    } else { // form is `exp => ...` or `[exp] => ...`
        exprs.push_back(tc->getArgs());
    }

    std::vector<kind::Kind*> argKinds(exprs.size());
    kind::Kind* retKind;

    for (size_t i = 0; i < exprs.size(); ++i) {
        argKinds[i] = exprs[i]->kind();
    }

    retKind = tc->getBody()->kind();

    tc->setKind(_mngr.New<kind::TypeConstructorKind>(argKinds, retKind));
}

void KindChecking::visit(TypeConstructorCall* tcall) {
    ASTImplicitVisitor::visit(tcall);

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
        _rep.error(*tcall->getCallee(), "Type expression " + tcall->getCallee()->kind()->toString() + " cannot be called.");
    }
}

void KindChecking::visit(TypeIdentifier* tident) {
    if (sym::Symbol* s = tident->getSymbol()) {
        if (kind::Kind* k = tryGetKindOfSymbol(s)) {
            tident->setKind(k);
        } else if (!_insideMemberAccess) {
            _rep.error(*tident, "Symbol " + s->getName() + " does not have any kind");
        }
    }
}

void KindChecking::visit(TypeToBeInferred* tbi) {
    tbi->setKind(kind::ProperKind::create());
}

void KindChecking::visit(KindSpecifier* ks) {
    ASTImplicitVisitor::visit(ks);
    kind::Kind* k = ASTKindCreator::createKind(ks->getKindNode(), _ctx);
    ks->getSpecified()->setKind(k);
    ks->setKind(k);
}

void KindChecking::visit(TypeSpecifier* ts) {
    ASTImplicitVisitor::visit(ts);

    if (ts->getTypeNode()->kind()->getKindGenre() != kind::KIND_PROPER) {
        _rep.error(*ts, "Variable cannot have type " + ts->getTypeNode()->kind()->toString() + " which is not a proper type.");
    }
}

void KindChecking::visit(Instantiation* inst) {
    ASTImplicitVisitor::visit(inst);

    if (inst->getInstantiatedExpression()->kind()->getKindGenre() != kind::KIND_PROPER) {
        _rep.error(*inst, "Can only instantiate proper types. Found " + inst->getInstantiatedExpression()->kind()->toString());
    }
}

void KindChecking::visitDeferredExpressions() {
    while (!_deferredExpressions.empty()) {
        std::set<TypeExpression*> copy = _deferredExpressions;
        _deferredExpressions.clear();

        for (TypeExpression* expr : copy) {
            _mustDefer = false;
            expr->onVisit(this);
        }
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
