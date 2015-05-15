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

TypeChecking::TypeChecking(CompCtx_Ptr& ctx, const sym::SymbolResolver& res)
    : ASTVisitor(ctx), _res(res), _rep(ctx->reporter()) {

}

TypeChecking::~TypeChecking() {

}

void TypeChecking::visit(ASTNode*) {

}

void TypeChecking::visit(ClassDecl* clss) {
    ASTVisitor::visit(clss);

    if (Expression* par = clss->getParent()) {
        if (type::Type* t = ASTTypeCreator::createType(par, _ctx)) {
            if (t->applied(_ctx)->getTypeKind() != type::TYPE_OBJECT) {
                _rep.error(*par, "Must inherit from a class");
            }
        }
    }
}

void TypeChecking::visit(DefineDecl* decl) {
    if (_visitedDefs.find(decl) == _visitedDefs.end()) {
        _visitedDefs.emplace(decl);

        decl->getValue()->onVisit(this);

        // type inference
        decl->getName()->setType(decl->getValue()->type());
        static_cast<sym::DefinitionSymbol*>(decl->getSymbol())->setType(decl->getValue()->type());
    }
}

void TypeChecking::visit(ExpressionStatement* exp) {
    ASTVisitor::visit(exp);
    exp->setType(exp->getExpression()->type());
}

void TypeChecking::visit(BinaryExpression* bin) {
    ASTVisitor::visit(bin);
    bin->setType(bin->getLhs()->type()); // TODO : make it right
}

void TypeChecking::visit(AssignmentExpression* aex) {
    ASTVisitor::visit(aex);

    type::Type* lhsT = aex->getLhs()->type();
    type::Type* rhsT = aex->getRhs()->type();

    if (!rhsT->applied(_ctx)->isSubTypeOf(lhsT->applied(_ctx))) {
        _rep.error(*aex, "Assigning incompatible type. Found " +
                   lhsT->toString() + ", expected " + rhsT->toString());
    }

    aex->setType(lhsT);
}

void TypeChecking::visit(TypeSpecifier* tps) {
    tps->getSpecified()->onVisit(this);
    tps->getTypeNode()->onVisit(this);

    if (type::Type* tpe = ASTTypeCreator::createType(tps->getTypeNode(), _ctx)) {
        Identifier* id = tps->getSpecified();
        type::Typed* tped = nullptr;

        if (id->getSymbol()->getSymbolType() == sym::SYM_VAR) {
            tped = static_cast<sym::VariableSymbol*>(id->getSymbol());
        } else if (id->getSymbol()->getSymbolType() == sym::SYM_DEF) {
            tped = static_cast<sym::DefinitionSymbol*>(id->getSymbol());
        }

        tped->setType(tpe);
    } else {
        _ctx->reporter().error(*tps->getTypeNode(), "Expression is not a type");
    }

    tps->setType(_res.Unit());
}

void TypeChecking::visit(Block* block) {
    ASTVisitor::visit(block);

    const std::vector<Expression*>& stats = block->getStatements();
    if (stats.size() > 0) {
        block->setType(stats.back()->type());
    } else {
        block->setType(_res.Unit());
    }
}

void TypeChecking::visit(IfExpression* ifexpr) {
    ASTVisitor::visit(ifexpr);

    if (!ifexpr->getCondition()->type()->applied(_ctx)->isSubTypeOf(_res.Bool())) {
        _rep.error(*ifexpr->getCondition(), "Condition is not a boolean (Found " + ifexpr->getCondition()->type()->toString() + ")");
    }

    if (ifexpr->getElse()) {
        type::Type* thenType = ifexpr->getThen()->type();
        type::Type* elseType = ifexpr->getElse()->type();

        if (thenType->applied(_ctx)->isSubTypeOf(elseType->applied(_ctx))) {
            ifexpr->setType(elseType);
        } else if (elseType->applied(_ctx)->isSubTypeOf(thenType->applied(_ctx))) {
            ifexpr->setType(thenType);
        } else {
            _rep.error(*ifexpr, "The then-part and else-part have different types. Found " +
                       thenType->toString() + " and " + elseType->toString());
        }

    } else {
        // TODO : correct this
        ifexpr->setType(ifexpr->getThen()->type());
    }
}

void TypeChecking::visit(MemberAccess* dot) {
    dot->getAccessed()->onVisit(this);

    if (type::Type* t = dot->getAccessed()->type()) {
        t = t->applied(_ctx);
        if (type::ObjectType* obj = type::getIf<type::ObjectType>(t)) {
            ClassDecl* clss = obj->getClass();
            const type::SubstitutionTable& subtable = obj->getSubstitutionTable();

            if (type::Type* tpe = tryGetTypeOfField(clss, dot->getMember()->getValue(), subtable)) {
                if (type::ObjectType* t = type::getIf<type::ObjectType>(tpe)) {
                    dot->setType(t);
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

void TypeChecking::visit(Tuple* tuple) {
    ASTVisitor::visit(tuple);
}

void TypeChecking::visit(FunctionCreation* func) {
    ASTVisitor::visit(func);

    _rep.info(*func->getArgs(), func->getBody()->type()->toString());

    func->setType(func->getBody()->type()); // TODO : change it
}

void TypeChecking::visit(FunctionCall* call) {
    ASTVisitor::visit(call);
    call->setType(call->getCallee()->type()->applyEnv(call->getCallee()->type()->getSubstitutionTable(), _ctx));
}

void TypeChecking::visit(Identifier* ident) {
    if (sym::Symbol* sym = ident->getSymbol()) {
        if (type::Type* t = tryGetTypeOfSymbol(sym)) {
            ident->setType(t);
        }
    }
}

void TypeChecking::visit(IntLitteral* intlit) {
    intlit->setType(_res.Int());
}

void TypeChecking::visit(RealLitteral* reallit) {
    reallit->setType(_res.Real());
}

type::Type* TypeChecking::tryGetTypeOfSymbol(sym::Symbol* sym) {
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

type::Type* TypeChecking::tryGetTypeOfField(ClassDecl* clss, const std::string& id, const type::SubstitutionTable& subtable) {
    if (sym::Symbol* sym = clss->getScope()->getSymbol<sym::Symbol>(id, false)) {
        return type::Type::findSubstitution(subtable, tryGetTypeOfSymbol(sym))->applyEnv(subtable, _ctx);
    } else if (Expression* parent = clss->getParent()) {
        if (type::Type* t = ASTTypeCreator::createType(parent, _ctx)) {
            type::Type* appliedT = type::Type::findSubstitution(subtable, t)->applyEnv(subtable, _ctx);
            if (type::ObjectType* obj = type::getIf<type::ObjectType>(appliedT)) {
                return tryGetTypeOfField(obj->getClass(), id, obj->getSubstitutionTable());
            } else {
                _rep.error(*parent, "Class " + clss->getName() + " must inherit from a class type");
            }
        } else {
            _rep.error(*parent, "Expression is not a type");
        }
    }
    return nullptr;
}

}

}
