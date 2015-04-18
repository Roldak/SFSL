//
//  ASTTypeCreator.cpp
//  SFSL
//
//  Created by Romain Beguet on 31.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "ASTTypeCreator.h"
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

void ASTTypeCreator::visit(ClassDecl *clss) {
    _created = _mngr.New<type::ObjectType>(clss);
}

void ASTTypeCreator::visit(TypeConstructorCreation* typeconstructor) {
    _created = _mngr.New<type::ConstructorType>(typeconstructor);
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

        type::SubstitutionTable subTable;

        for (size_t i = 0; i < params.size(); ++i) {
            sym::TypeSymbol* param = static_cast<sym::TypeSymbol*>(static_cast<Identifier*>(params[i])->getSymbol());
            args[i]->onVisit(this);
            subTable[param->type()] = _created;
        }

        constructor->getBody()->onVisit(this);

        if (_created) {
            if (type::ObjectType* obj = type::getIf<type::ObjectType>(_created)) {
                _created = substituteTypes(obj, subTable);
            } else {
                _ctx.get()->reporter().error(*tcall, "wtf");
            }
        } else {
            _ctx.get()->reporter().fatal(*tcall, "Type instantiation failed");
        }

    } else {
        _ctx.get()->reporter().error(*tcall, "type expression is not a type constructor");
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

type::ObjectType* ASTTypeCreator::substituteTypes(type::ObjectType* original, const type::SubstitutionTable& table) {
    return _mngr.New<type::ObjectType>(original->getClass(), table);
}

}

}
