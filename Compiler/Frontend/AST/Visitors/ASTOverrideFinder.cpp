//
//  ASTOverrideFinder.cpp
//  SFSL
//
//  Created by Romain Beguet on 27.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "ASTOverrideFinder.h"
#include "ASTTypeCreator.h"

namespace sfsl {

namespace ast {

ASTOverrideFinder::ASTOverrideFinder(CompCtx_Ptr& ctx, sym::DefinitionSymbol* overridingSymbol)
    : ASTVisitor(ctx), _overridingSymbol(overridingSymbol), _overridenSymbol(nullptr),
      _currentType(static_cast<type::ProperType*>(ASTTypeCreator::createType(overridingSymbol->getOwner(), ctx)->applied(_ctx))) {

}

ASTOverrideFinder::~ASTOverrideFinder() {

}

void ASTOverrideFinder::visit(ASTNode*) {
    // do not throw an exception
}


void ASTOverrideFinder::visit(ClassDecl* clss){
    for (DefineDecl* decl : clss->getDefs()) {
        if (    _overridingSymbol->getName() == decl->getName()->getValue() &&
                _overridingSymbol != decl->getSymbol() &&
                _overridingSymbol->type()->isSubTypeOf(decl->getSymbol()->type()->applyEnv(_currentType->getSubstitutionTable(), _ctx))) {

            if (decl->isRedef()) {
                if (sym::DefinitionSymbol* alreadyOverriden = decl->getSymbol()->getOverridenSymbol()) {
                    _overridenSymbol = alreadyOverriden;
                    return;
                } else {
                    break;
                }
            } else {
                _overridenSymbol = decl->getSymbol();
                return;
            }
        }
    }

    if (clss->getParent()) {
        SAVE_MEMBER_AND_SET(
                    _currentType,
                    static_cast<type::ProperType*>(
                        ASTTypeCreator::createType(clss->getParent(), _ctx)->applyEnv(_currentType->getSubstitutionTable(), _ctx)))

        _currentType->getClass()->onVisit(this);

        RESTORE_MEMBER(_currentType)
    }
}

sym::DefinitionSymbol* ASTOverrideFinder::getOverridenSymbol() const {
    return _overridenSymbol;
}

sym::DefinitionSymbol* ASTOverrideFinder::findOverridenSymbol(sym::DefinitionSymbol* node, CompCtx_Ptr& ctx) {
    ASTOverrideFinder ovrf(ctx, node);
    node->getOwner()->onVisit(&ovrf);
    return ovrf.getOverridenSymbol();
}

}

}
