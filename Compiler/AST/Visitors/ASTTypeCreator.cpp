//
//  ASTTypeCreator.cpp
//  SFSL
//
//  Created by Romain Beguet on 31.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "ASTTypeCreator.h"
#include "ASTTypeIdentifier.h"
#include "../Symbols/Scope.h"
#include "../Symbols/SymbolResolver.h"
#include "../../Analyser/NameAnalysis.h"
#include "../../Analyser/TypeChecking.h"
#include "../../../Utils/Utils.h"

namespace sfsl {

namespace ast {

ASTTypeCreator::ASTTypeCreator(CompCtx_Ptr& ctx, const type::SubstitutionTable& subTable)
    : ASTVisitor(ctx), _created(nullptr), _subTable(subTable) {

}

ASTTypeCreator::~ASTTypeCreator() {

}

void ASTTypeCreator::visit(ASTNode* node) {
    // do not throw an exception
}

void ASTTypeCreator::visit(ClassDecl* clss) {
    _created = _mngr.New<type::ObjectType>(clss, _subTable);
}

void ASTTypeCreator::visit(TypeConstructorCreation* typeconstructor) {
    _created = _mngr.New<type::ConstructorType>(typeconstructor, _subTable);
}

void ASTTypeCreator::visit(TypeConstructorCall *tcall) {
    tcall->getCallee()->onVisit(this);

    type::Type* ctr = _created;

    if (type::Type* tmp = ctr->applyEnv({}, _ctx)) {
        if (tmp->getTypeKind() != type::TYPE_CONSTRUCTOR) {
            _ctx.get()->reporter().error(*tcall, "Expression is not a type constructor.");
        }
    } else {
        _ctx.get()->reporter().fatal(*tcall, "Failed to create a type");
    }

    // TODO : Check args kind

    std::vector<type::Type*> args;
    for (size_t i = 0; i < tcall->getArgs().size(); ++i) {
        tcall->getArgs()[i]->onVisit(this);
        args.push_back(_created);
    }

    _created = _mngr.New<type::ConstructorApplyType>(ctr, args, *tcall, _subTable);

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

void ASTTypeCreator::createTypeFromSymbolic(sym::Symbolic<sym::Symbol>* symbolic, common::Positionnable& pos) {
    if (sym::Symbol* symbol = symbolic->getSymbol()) {
        if (symbol->getSymbolType() != sym::SYM_TPE) {
            _ctx.get()->reporter().error(pos, "Symbol " + symbol->getName() + " does not refer a type");
            return;
        }

        sym::TypeSymbol* ts = static_cast<sym::TypeSymbol*>(symbol);

        if (ts->type() == type::Type::NotYetDefined()) {

            if (_visitedTypes.find(ts) == _visitedTypes.end()) {
                _visitedTypes.emplace(ts);

                ts->getTypeDecl()->getExpression()->onVisit(this);
                ts->setType(_created);
            } else {
                _ctx.get()->reporter().error(pos, "A cyclic dependency was found");
            }

        } else {
            _created = ts->type();
        }
    }
}

}

}
