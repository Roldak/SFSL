//
//  ASTKindCreator.cpp
//  SFSL
//
//  Created by Romain Beguet on 14.05.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "ASTKindCreator.h"
#include "../../Symbols/Scope.h"
#include "ASTTypeCreator.h"

namespace sfsl {

namespace ast {

// AST KIND CREATOR

ASTKindCreator::ASTKindCreator(CompCtx_Ptr& ctx)
    : ASTImplicitVisitor(ctx), _created(nullptr) {

}

ASTKindCreator::~ASTKindCreator() {

}

void ASTKindCreator::visit(ASTNode* node) {
    // do not throw an exception
}

type::Type* ASTKindCreator::computeBoundType(TypeExpression* b) {
    if (b) {
        if (type::Type* tp = ASTTypeCreator::createType(b, _ctx)) {
            if (tp->applyTCCallsOnly(_ctx)->getTypeKind() == type::TYPE_PROPER) {
                return tp;
            } else {
                _ctx->reporter().error(*b, "Proper kind can only have bounds of a proper type");
            }
        }
    }
    return nullptr;
}

void ASTKindCreator::visit(ProperTypeKindSpecifier* ptks) {
    type::Type* lbType = computeBoundType(ptks->getLowerBoundExpr());
    type::Type* ubType = computeBoundType(ptks->getUpperBoundExpr());

    _created = (!lbType && !ubType) ? kind::ProperKind::create()
                                    : _mngr.New<kind::ProperKind>(lbType, ubType);
}

void ASTKindCreator::visit(TypeConstructorKindSpecifier* tcks) {
    std::vector<kind::TypeConstructorKind::Parameter> params(tcks->getArgs().size());
    kind::Kind* ret;

    for (size_t i = 0; i < params.size(); ++i) {
        const TypeConstructorKindSpecifier::Parameter& tcParam(tcks->getArgs()[i]);
        tcParam.kindExpr->onVisit(this);

        if (!_created) {
            return;
        }

        params[i].varianceType = tcParam.varianceType;
        params[i].kind = _created;
    }

    tcks->getRet()->onVisit(this);

    if (!(ret = _created)) {
        return;
    }

    _created = _mngr.New<kind::TypeConstructorKind>(params, ret);
}

kind::Kind* ASTKindCreator::getCreatedKind() const {
    return _created;
}

kind::Kind* ASTKindCreator::createKind(ASTNode* node, CompCtx_Ptr& ctx) {
    ASTKindCreator creator(ctx);
    node->onVisit(&creator);
    return creator.getCreatedKind();
}

}

}
