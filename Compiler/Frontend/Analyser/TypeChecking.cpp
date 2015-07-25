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
#include "../AST/Visitors/ASTAssignmentChecker.h"
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

void TypeChecking::visit(TypeDecl* tdecl) {
    tdecl->setType(_res.Unit());

    ASTVisitor::visit(tdecl);
}

void TypeChecking::visit(DefineDecl* decl) {
    decl->setType(_res.Unit());

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

    if (!ASTAssignmentChecker::isExpressionAssignable(aex->getLhs(), _ctx)) {
        _rep.error(*aex, "Left hand side is not an assignable expression");
    }

    type::Type* lhsT = aex->getLhs()->type();
    type::Type* rhsT = aex->getRhs()->type();

    if (!rhsT->applied(_ctx)->isSubTypeOf(lhsT->applied(_ctx))) {
        _rep.error(*aex, "Assigning incompatible type. Found " +
                   rhsT->toString() + ", expected " + lhsT->toString());
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
        tps->setType(tpe);
    } else {
        _ctx->reporter().error(*tps->getTypeNode(), "Expression is not a type");
    }
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
        if (type::ProperType* obj = type::getIf<type::ProperType>(t->applied(_ctx))) {
            ClassDecl* clss = obj->getClass();
            const type::SubstitutionTable& subtable = obj->getSubstitutionTable();

            FieldInfo field = tryGetFieldInfo(clss, dot->getMember()->getValue(), subtable);

            if (field.s && field.t) {
                dot->setSymbol(field.s);

                if (type::ProperType* t = type::getIf<type::ProperType>(field.t)) {
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

    Expression* expr = func->getArgs();
    std::vector<Expression*> args;

    if (isNodeOfType<Tuple>(expr, _ctx)) { // form is `() => ...` or `(exp, exp) => ...`, ...
        args = static_cast<Tuple*>(expr)->getExpressions();
    } else { // form is `exp => ...` or `(exp) => ...`
        args.push_back(expr);
    }

    std::vector<type::Type*> argTypes(args.size());
    type::Type* retType = func->getBody()->type();

    for (size_t i = 0; i < args.size(); ++i) {
        argTypes[i] = args[i]->type();
    }

    func->setType(_mngr.New<type::FunctionType>(argTypes, retType, nullptr));


    _rep.info(*func->getArgs(), func->type()->toString());
}

void TypeChecking::visit(FunctionCall* call) {
    ASTVisitor::visit(call);

    type::Type* calleeT = call->getCallee()->type();

    const std::vector<Expression*>& callArgs = call->getArgs();
    std::vector<type::Type*> callArgTypes(callArgs.size());

    for (size_t i = 0; i < callArgs.size(); ++i) {
        callArgTypes[i] = callArgs[i]->type();
    }

    if (type::FunctionType* ft = type::getIf<type::FunctionType>(calleeT)) {
        const std::vector<type::Type*>& expectedArgTypes = ft->getArgTypes();

        if (callArgTypes.size() != expectedArgTypes.size()) {
            _rep.error(*call->getArgsTuple(),
                       "Wrong number of argument. Found " + utils::T_toString(callArgTypes.size()) +
                       ", expected " + utils::T_toString(expectedArgTypes.size()));
            return;
        }

        for (size_t i = 0; i < expectedArgTypes.size(); ++i) {
            if (!callArgTypes[i]->isSubTypeOf(expectedArgTypes[i])) {
                _rep.error(*callArgs[i],
                           "Argument type mismatch. Found " + callArgTypes[i]->toString() +
                           ", expected " + expectedArgTypes[i]->toString());
            }
        }

        call->setType(ft->getRetType());

    } else {
        _rep.error(*call, "Expression is not callable");
    }
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

TypeChecking::FieldInfo TypeChecking::tryGetFieldInfo(ClassDecl* clss, const std::string& id, const type::SubstitutionTable& subtable) {
    if (sym::Symbol* sym = clss->getScope()->getSymbol<sym::Symbol>(id, false)) {
        return {sym, type::Type::findSubstitution(subtable, tryGetTypeOfSymbol(sym))->applyEnv(subtable, _ctx)};
    } else if (TypeExpression* parent = clss->getParent()) {
        if (type::Type* t = ASTTypeCreator::createType(parent, _ctx)) {
            type::Type* appliedT = type::Type::findSubstitution(subtable, t)->applyEnv(subtable, _ctx);
            if (type::ProperType* obj = type::getIf<type::ProperType>(appliedT)) {
                return tryGetFieldInfo(obj->getClass(), id, obj->getSubstitutionTable());
            } else {
                _rep.error(*parent, "Class " + clss->getName() + " must inherit from a class type");
            }
        } else {
            _rep.error(*parent, "Expression is not a type");
        }
    }
    return {nullptr, nullptr};
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

// FIELD INFO

TypeChecking::FieldInfo::FieldInfo(sym::Symbol* sy, type::Type* ty) : s(sy), t(ty) {

}

}

}
