//
//  ASTKindCreator.cpp
//  SFSL
//
//  Created by Romain Beguet on 14.05.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "ASTKindCreator.h"

namespace sfsl {

namespace ast {

ASTKindCreator::ASTKindCreator(CompCtx_Ptr& ctx)
    : ASTVisitor(ctx), _created(nullptr) {

}

ASTKindCreator::~ASTKindCreator() {

}

void ASTKindCreator::visit(ASTNode* node) {
    // do not throw an exception
}

void ASTKindCreator::visit(ProperTypeKindSpecifier* ptks) {
    _created = kind::TypeKind::create();
}

void ASTKindCreator::visit(TypeConstructorKindSpecifier* tcks) {
    std::vector<kind::Kind*> args(tcks->getArgs().size());
    kind::Kind* ret;

    for (size_t i = 0; i < args.size(); ++i) {
        tcks->getArgs()[i]->onVisit(this);
        args[i] = _created;
    }

    tcks->getRet()->onVisit(this);
    ret = _created;

    _created = _mngr.New<kind::TypeConstructorKind>(args, ret);
}

kind::Kind* ASTKindCreator::getCreatedKind() const {
    return _created;
}

}

}
