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
        if (type::ConstructorType* constr = type::getIf<type::ConstructorType>(tmp)) {
            /*
            const std::vector<TypeExpression*>& found = tcall->getArgs();
            const std::vector<TypeExpression*>& expec = constr->getTypeConstructor()->getArgs()->getExpressions();

            if (found.size() != expec.size()) {
                _ctx->reporter().error(*tcall, "Wrong number of arguments. Found " +
                                             utils::T_toString(found.size()) + " Expected " + utils::T_toString(expec.size()));
            }

            std::vector<type::Type*> args(found.size());

            for (size_t i = 0; i < found.size(); ++i) {
                if (!(args[i] = kindCheck(expec[i], found[i]))) {
                    _created = nullptr;
                    return;
                }
            }

            _created = _mngr.New<type::ConstructorApplyType>(ctr, args, *tcall, _subTable);
            */

        } else {
            _ctx->reporter().error(*tcall, "Expression is not a type constructor.");
        }
    } else {
        _ctx->reporter().fatal(*tcall, "Failed to create a type");
    }
}

void ASTTypeCreator::visit(TypeMemberAccess* mac) {
    createTypeFromSymbolic(mac, *mac);
}

void ASTTypeCreator::visit(TypeIdentifier* ident) {
    createTypeFromSymbolic(ident, *ident);
}

type::Type* ASTTypeCreator::getCreatedType() const {
    return _created;
}

void ASTTypeCreator::createTypeFromSymbolic(sym::Symbolic<sym::Symbol>* symbolic, common::Positionnable& pos) {
    if (sym::Symbol* s = symbolic->getSymbol()) {
        if (s->getSymbolType() != sym::SYM_TPE) {
            _ctx->reporter().error(pos, "Expression is not a type");
        }

        sym::TypeSymbol* ts = static_cast<sym::TypeSymbol*>(s);

        if (ts->type() == type::Type::NotYetDefined()) {

            if (_visitedTypes.find(ts) == _visitedTypes.end()) {
                _visitedTypes.emplace(ts);

                ts->getTypeDecl()->getExpression()->onVisit(this);
                ts->setType(_created);
            } else {
                _ctx->reporter().error(pos, "A cyclic dependency was found");
            }

        } else {
            _created = ts->type();
        }
    }
}

}

}
