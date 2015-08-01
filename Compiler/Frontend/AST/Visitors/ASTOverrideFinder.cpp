//
//  ASTOverrideFinder.cpp
//  SFSL
//
//  Created by Romain Beguet on 27.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "ASTOverrideFinder.h"
#include "ASTTypeCreator.h"
#include "../Symbols/Scope.h"

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
    const std::multimap<std::string, sym::SymbolData>& symbols(clss->getScope()->getAllSymbols());
    const auto& itPair = symbols.equal_range(_overridingSymbol->getName());

    for (auto it = itPair.first; it != itPair.second; ++it) {
        const sym::SymbolData& data = it->second;
        if (it->second.symbol != _overridingSymbol && data.symbol->getSymbolType() == sym::SYM_DEF) {
            sym::DefinitionSymbol* def = static_cast<sym::DefinitionSymbol*>(data.symbol);
            if (_overridingSymbol->type()->isSubTypeOf(def->type()->applyEnv(data.env, _ctx))) {
                if (def->getDef()->isRedef()) {
                    if (sym::DefinitionSymbol* alreadyOverriden = def->getOverridenSymbol()) {
                        _overridenSymbol = alreadyOverriden;
                        return;
                    } else {
                        continue;
                    }
                } else {
                    _overridenSymbol = def;
                    return;
                }
            }
        }
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
