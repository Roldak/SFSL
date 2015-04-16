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

TypeCheking::TypeCheking(CompCtx_Ptr& ctx, const sym::SymbolResolver& res)
    : ASTVisitor(ctx), _curScope(nullptr), _res(res), _rep(ctx.get()->reporter()) {

}

void TypeCheking::visit(ASTNode*) {

}

void TypeCheking::visit(ModuleDecl* mod) {
    SAVE_SCOPE(mod->getSymbol())

    ASTVisitor::visit(mod);

    RESTORE_SCOPE
}

void TypeCheking::visit(ClassDecl* clss) {
    SAVE_SCOPE(clss->getSymbol())

    ASTVisitor::visit(clss);

    RESTORE_SCOPE
}

void TypeCheking::visit(DefineDecl* decl) {
    SAVE_SCOPE(decl->getSymbol())

    ASTVisitor::visit(decl);

    // type inference
    decl->getName()->setType(decl->getValue()->type());
    static_cast<sym::DefinitionSymbol*>(decl->getSymbol())->setType(decl->getValue()->type());

    RESTORE_SCOPE
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
                   aex->getRhs()->type()->toString() + ", Expected " + aex->getLhs()->type()->toString());
    }

    aex->setType(aex->getLhs()->type());
}

void TypeCheking::visit(TypeSpecifier* tps) {
    tps->getSpecified()->onVisit(this);

    if (type::Type* tpe = createType(tps->getTypeNode(), _ctx)) {
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
            _rep.error(*ifexpr, "The then-part and else-part have different types (" +
                       thenType->toString() + " and " + elseType->toString() + " found)");
        }

    } else {
        ifexpr->setType(ifexpr->getThen()->type());
    }
}

void TypeCheking::visit(MemberAccess* dot) {
    dot->getAccessed()->onVisit(this);

    if (type::Type* t = dot->getAccessed()->type()) {
        if (t->getTypeKind() == type::TYPE_OBJECT) {
            sym::ClassSymbol* clss = static_cast<type::ObjectType*>(t)->getClass();
            if (sym::Symbol* sym = clss->getScope()->getSymbol<sym::Symbol>(dot->getMember()->getValue(), false)) {

                if (sym->getSymbolType() == sym::SYM_VAR) {
                    dot->setType(static_cast<sym::VariableSymbol*>(sym)->type());
                } else if (sym->getSymbolType() == sym::SYM_DEF) {
                    dot->setType(static_cast<sym::DefinitionSymbol*>(sym)->type());
                } else { // is not supposed to happen
                    _rep.error(*dot->getMember(), "member " + dot->getMember()->getValue() +
                               " of class " + clss->getName() + " is not a value");
                }

            } else {
                _rep.error(*dot->getMember(), "no member named " +
                           dot->getMember()->getValue() + " in class " + clss->getName());
            }
        }
    }
}

void TypeCheking::visit(Tuple* tuple) {
    ASTVisitor::visit(tuple);
}

void TypeCheking::visit(FunctionCreation* func) {
    SAVE_SCOPE(func)

    ASTVisitor::visit(func);

    _rep.info(*func, func->getBody()->type()->toString());

    RESTORE_SCOPE
}

void TypeCheking::visit(FunctionCall* call) {
    ASTVisitor::visit(call);
}

void TypeCheking::visit(Identifier* ident) {
    if (sym::Symbol* sym = ident->getSymbol()) {
        if (sym->getSymbolType() == sym::SYM_VAR) {
            ident->setType(static_cast<sym::VariableSymbol*>(ident->getSymbol())->type());
        } else if (sym->getSymbolType() == sym::SYM_DEF) {
            ident->setType(static_cast<sym::DefinitionSymbol*>(ident->getSymbol())->type());
        }
    }
}

void TypeCheking::visit(IntLitteral* intlit) {
    intlit->setType(_res.Int());
}

void TypeCheking::visit(RealLitteral* reallit) {
    reallit->setType(_res.Real());
}

}

}
