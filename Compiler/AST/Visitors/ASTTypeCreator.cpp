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

namespace sfsl {

namespace ast {

ASTTypeCreator::ASTTypeCreator(CompCtx_Ptr& ctx, const sym::SymbolResolver* res)
    : ASTVisitor(ctx), _created(nullptr), _res(res) {

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
    if (_created->getTypeKind() == type::TYPE_CONSTRUCTOR) {
        TypeConstructorCreation* constructor = static_cast<type::ConstructorType*>(_created)->getTypeConstructor();

        const std::vector<Expression*>& args = tcall->getArgs();
        const std::vector<Expression*>& params = constructor->getArgs()->getExpressions();

        if (args.size() != params.size()) {
            _ctx.get()->reporter().error(*tcall, "Expected argument count is different from what was found");
            _created = nullptr;
            return;
        }

        const type::SubstitutionTable old = _subTable;
        _subTable = type::SubstitutionTable();

        for (size_t i = 0; i < params.size(); ++i) {
            sym::TypeSymbol* param = nullptr;
            if (isNodeOfType<Identifier>(params[i], _ctx)) {
                param = static_cast<sym::TypeSymbol*>(static_cast<Identifier*>(params[i])->getSymbol());
            } else if (isNodeOfType<TypeConstructorCall>(params[i], _ctx)) {
                param = static_cast<sym::TypeSymbol*>(
                            static_cast<Identifier*>(
                                static_cast<TypeConstructorCall*>(
                                    params[i]
                                    )->getCallee()
                                )->getSymbol()
                            );
            }

            args[i]->onVisit(this);
            _subTable[param->type()] = _created;
        }

        constructor->getBody()->onVisit(this);

        _subTable = old;

        if (!_created) {
            _ctx.get()->reporter().fatal(*tcall, "Type instantiation failed");
        }

    } else {
        _ctx.get()->reporter().error(*tcall, "Expression is not a type constructor");
    }
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
