//
//  TypeChecking.cpp
//  SFSL
//
//  Created by Romain Beguet on 30.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "TypeChecking.h"
#include "../AST/Visitors/ASTTypeIdentifier.h"
#include "../AST/Visitors/ASTTypeCreator.h"
#include "../AST/Symbols/Scope.h"

namespace sfsl {

namespace ast {

// TYPE CHECK

TypeCheking::TypeCheking(CompCtx_Ptr& ctx, const sym::SymbolResolver& res, sym::Scope *initialScope)
    : ASTVisitor(ctx), _curScope(initialScope), _res(res), _rep(ctx.get()->reporter()) {

}

void TypeCheking::visit(ASTNode*) {

}

void TypeCheking::visit(ModuleDecl* mod) {
    SAVE_SCOPE(mod->getSymbol())

    ASTVisitor::visit(mod);

    RESTORE_SCOPE
}

void TypeCheking::visit(TypeDecl *tdecl) {
    ASTVisitor::visit(tdecl);
    tdecl->setType(_res.Unit());
}

void TypeCheking::visit(ClassDecl* clss) {
    SAVE_SCOPE(clss)

    ASTVisitor::visit(clss);

    RESTORE_SCOPE
}

void TypeCheking::visit(DefineDecl* decl) {
    if (_visitedDefs.find(decl) == _visitedDefs.end()) {
        _visitedDefs.emplace(decl);

        SAVE_SCOPE(decl->getSymbol())

        decl->getValue()->onVisit(this);

        // type inference
        decl->getName()->setType(decl->getValue()->type());
        static_cast<sym::DefinitionSymbol*>(decl->getSymbol())->setType(decl->getValue()->type());

        RESTORE_SCOPE
    }
}

void TypeCheking::visit(TypeTuple* ttuple) {

}

void TypeCheking::visit(TypeConstructorCreation* typeconstructor) {
    SAVE_SCOPE(typeconstructor)

    ASTVisitor::visit(typeconstructor);

    RESTORE_SCOPE
}

void TypeCheking::visit(TypeConstructorCall* tcall) {

}

void TypeCheking::visit(ExpressionStatement* exp) {
    ASTVisitor::visit(exp);
    exp->setType(exp->getExpression()->type());
}

void TypeCheking::visit(BinaryExpression* bin) {
    ASTVisitor::visit(bin);
    bin->setType(bin->getLhs()->type()); // TODO : make it right
}

void TypeCheking::visit(AssignmentExpression* aex) {
    ASTVisitor::visit(aex);

    if (!aex->getRhs()->type()->isSubTypeOf(aex->getLhs()->type())) {
        _rep.error(*aex, "Assigning incompatible type. Found " +
                   aex->getRhs()->type()->toString() + ", expected " + aex->getLhs()->type()->toString());
    }

    aex->setType(aex->getLhs()->type());
}

void TypeCheking::visit(TypeSpecifier* tps) {
    tps->getSpecified()->onVisit(this);
    tps->getTypeNode()->onVisit(this);

    if (type::Type* tpe = createType(tps->getTypeNode(), _ctx, &_res)) {
        Identifier* id = tps->getSpecified();
        type::Typed* tped = nullptr;

        if (id->getSymbol()->getSymbolType() == sym::SYM_VAR) {
            tped = static_cast<sym::VariableSymbol*>(id->getSymbol());
        } else if (id->getSymbol()->getSymbolType() == sym::SYM_DEF) {
            tped = static_cast<sym::DefinitionSymbol*>(id->getSymbol());
        }

        tped->setType(tpe);
    } else {
        _ctx.get()->reporter().error(*tps->getTypeNode(), "Expression is not a type");
    }

    tps->setType(_res.Unit());
}

void TypeCheking::visit(Block* block) {
    SAVE_SCOPE(block)

    ASTVisitor::visit(block);

    const std::vector<Expression*>& stats = block->getStatements();
    if (stats.size() > 0) {
        block->setType(stats.back()->type());
    } else {
        block->setType(_res.Unit());
    }

    RESTORE_SCOPE
}

void TypeCheking::visit(IfExpression* ifexpr) {
    ASTVisitor::visit(ifexpr);

    if (!ifexpr->getCondition()->type()->isSubTypeOf(_res.Bool())) {
        _rep.error(*ifexpr->getCondition(), "Condition is not a boolean (" + ifexpr->getCondition()->type()->toString() + ")");
    }

    if (ifexpr->getElse()) {
        type::Type* thenType = ifexpr->getThen()->type();
        type::Type* elseType = ifexpr->getElse()->type();

        if (thenType->isSubTypeOf(elseType)) {
            ifexpr->setType(elseType);
        } else if (elseType->isSubTypeOf(thenType)) {
            ifexpr->setType(thenType);
        } else {
            _rep.error(*ifexpr, "The then-part and else-part have different types. Found " +
                       thenType->toString() + " and " + elseType->toString());
        }

    } else {
        ifexpr->setType(ifexpr->getThen()->type());
    }
}

void TypeCheking::visit(MemberAccess* dot) {
    dot->getAccessed()->onVisit(this);

    if (type::Type* t = dot->getAccessed()->type()) {
        if (type::ObjectType* obj = type::getIf<type::ObjectType>(t)) {
            ClassDecl* clss = obj->getClass();

            if (sym::Symbol* sym = clss->getScope()->getSymbol<sym::Symbol>(dot->getMember()->getValue(), false)) {
                if (type::ObjectType* t = type::getIf<type::ObjectType>(obj->trySubstitution(tryGetTypeOfSymbol(sym)))) {
                    dot->setType(applySubsitutions(t, obj));
                } else {
                    _rep.error(*dot->getMember(), "Member " + dot->getMember()->getValue() +
                               " of class " + clss->getName() + " is not a value");
                }
            } else {
                _rep.error(*dot->getMember(), "No member named " + dot->getMember()->getValue() +
                           " in class " + clss->getName());
            }
        }
    }

    // TODO : static access
}

void TypeCheking::visit(Tuple* tuple) {
    ASTVisitor::visit(tuple);
}

void TypeCheking::visit(FunctionCreation* func) {
    SAVE_SCOPE(func)

    ASTVisitor::visit(func);

    _rep.info(*func->getArgs(), func->getBody()->type()->toString());

    func->setType(func->getBody()->type()); // TODO : change it

    RESTORE_SCOPE
}

void TypeCheking::visit(FunctionCall* call) {
    ASTVisitor::visit(call);
    call->setType(call->getCallee()->type());
}

void TypeCheking::visit(Identifier* ident) {
    if (sym::Symbol* sym = ident->getSymbol()) {
        if (type::Type* t = tryGetTypeOfSymbol(sym)) {
            ident->setType(t);
        }
    }
}

void TypeCheking::visit(IntLitteral* intlit) {
    intlit->setType(_res.Int());
}

void TypeCheking::visit(RealLitteral* reallit) {
    reallit->setType(_res.Real());
}

type::Type* TypeCheking::tryGetTypeOfSymbol(sym::Symbol* sym) {
    if (sym->getSymbolType() == sym::SYM_VAR) {
        return static_cast<sym::VariableSymbol*>(sym)->type();
    } else if (sym->getSymbolType() == sym::SYM_DEF) {
        sym::DefinitionSymbol* defsym = static_cast<sym::DefinitionSymbol*>(sym);
        defsym->getDef()->onVisit(this);

        if (defsym->type() == type::Type::NotYetDefined()) {
            _rep.error(*defsym, "Type of " + defsym->getName() + " cannot be inferred because of a cyclic dependency");
        }

        return defsym->type();
    }
    return nullptr;
}

type::ObjectType* TypeCheking::applySubsitutions(type::ObjectType *inner, type::ObjectType *obj) {
    const type::SubstitutionTable& toSub = inner->getSubstitutionTable();
    type::SubstitutionTable newTable;

    for (const auto& pair : toSub) {
        type::Type* res = obj->trySubstitution(pair.second);
        if (type::ObjectType* o = type::getIf<type::ObjectType>(res)) {
            res = applySubsitutions(o, obj);
        }
        newTable[pair.first] = res;
    }

    return _mngr.New<type::ObjectType>(inner->getClass(), newTable);
}

}

}
