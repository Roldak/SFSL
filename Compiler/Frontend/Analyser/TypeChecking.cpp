//
//  TypeChecking.cpp
//  SFSL
//
//  Created by Romain Beguet on 30.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include <algorithm>

#include "TypeChecking.h"
#include "../AST/Visitors/ASTTypeIdentifier.h"
#include "../AST/Visitors/ASTTypeCreator.h"
#include "../AST/Visitors/ASTAssignmentChecker.h"
#include "../AST/Symbols/Scope.h"

namespace sfsl {

namespace ast {

// TYPE CHECKER

TypeChecker::TypeChecker(CompCtx_Ptr& ctx, const sym::SymbolResolver& res) : ASTVisitor(ctx), _res(res), _rep(ctx->reporter()) {

}

TypeChecker::~TypeChecker() {

}

// TOP LEVEL TYPE CHECKING

TopLevelTypeChecking::TopLevelTypeChecking(CompCtx_Ptr& ctx, const sym::SymbolResolver& res) : TypeChecker(ctx, res) {

}

TopLevelTypeChecking::~TopLevelTypeChecking() {

}

void TopLevelTypeChecking::visit(ASTNode* node) {

}

void TopLevelTypeChecking::visit(ClassDecl* clss) {
    ASTVisitor::visit(clss);

    for (TypeSpecifier* tps : clss->getFields()) {
        if (type::Type* tpe = ASTTypeCreator::createType(tps->getTypeNode(), _ctx)) {
            static_cast<sym::VariableSymbol*>(tps->getSpecified()->getSymbol())->setType(tpe);
            tps->setType(tpe);
        }
    }
}

void TopLevelTypeChecking::visit(DefineDecl* def) {
    _nextDef = def->getValue();
    def->getValue()->onVisit(this);
    def->getSymbol()->setType(def->getValue()->type());
}

void TopLevelTypeChecking::visit(FunctionCreation* func) {
    if (func == _nextDef) {
        // to be able to resolve overloads without having to fully typecheck the body of the function

        Expression* expr = func->getArgs();
        std::vector<Expression*> args;

        if (isNodeOfType<Tuple>(expr, _ctx)) { // form is `() => ...` or `(exp, exp) => ...`, ...
            args = static_cast<Tuple*>(expr)->getExpressions();
        } else { // form is `exp => ...` or `(exp) => ...`
            args.push_back(expr);
        }

        std::vector<type::Type*> argTypes(args.size());

        for (size_t i = 0; i < args.size(); ++i) {
            if (isNodeOfType<TypeSpecifier>(args[i], _ctx)) {
                TypeSpecifier* tps = static_cast<TypeSpecifier*>(args[i]);
                argTypes[i] = ASTTypeCreator::createType(tps->getTypeNode(), _ctx);
            } else {
                _rep.error(*args[i], "Omitting the type of the argument is forbidden in this case");
                argTypes[i] = type::Type::NotYetDefined();
            }
        }

        func->setType(_mngr.New<type::FunctionType>(argTypes, type::Type::NotYetDefined(), nullptr));
    }

    ASTVisitor::visit(func);
}

// TYPE CHECKING

TypeChecking::TypeChecking(CompCtx_Ptr& ctx, const sym::SymbolResolver& res)
    : TypeChecker(ctx, res), _currentThis(nullptr), _nextDef(nullptr), _expectedInfo{nullptr, nullptr, nullptr} {

}

TypeChecking::~TypeChecking() {

}

void TypeChecking::visit(ASTNode*) {

}

void TypeChecking::visit(Program* prog) {
    ASTVisitor::visit(prog);

    for (DefineDecl* redef : _redefs) {
        redef->getSymbol()->setOverridenSymbol(findOverridenSymbol(redef->getSymbol()));
    }
}

void TypeChecking::visit(TypeDecl* tdecl) {
    tdecl->setType(_res.Unit());

    ASTVisitor::visit(tdecl);
}

void TypeChecking::visit(DefineDecl* decl) {
    decl->setType(_res.Unit());

    if (TRY_INSERT(_visitedDefs, decl)) {
        SAVE_MEMBER_AND_SET(_currentThis, decl->getSymbol()->getOwner())
        SAVE_MEMBER_AND_SET(_nextDef, decl->getValue())

        decl->getValue()->onVisit(this);

        RESTORE_MEMBER(_nextDef)
        RESTORE_MEMBER(_currentThis)

        // type inference
        decl->getName()->setType(decl->getValue()->type());
        decl->getSymbol()->setType(decl->getValue()->type());

        if (decl->isRedef()) {
            _redefs.push_back(decl);
        }
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
    ASTVisitor::visit(tps);

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

    type::Type* thenType = ifexpr->getThen()->type();

    if (ifexpr->getElse()) {
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
        if (!thenType->applied(_ctx)->isSubTypeOf(_res.Unit())) {
            _rep.error(*ifexpr->getThen(), "An if-expression without else-part must have its then-part evaluate to unit. Found " +
                       thenType->toString());
        }
        ifexpr->setType(_res.Unit());
    }
}

void TypeChecking::visit(MemberAccess* dot) {
    dot->getAccessed()->onVisit(this);

    if (type::Type* t = dot->getAccessed()->type()) {
        if (type::ProperType* obj = type::getIf<type::ProperType>(t->applied(_ctx))) {
            ClassDecl* clss = obj->getClass();
            const type::SubstitutionTable& subtable = obj->getSubstitutionTable();

            FieldInfo field = tryGetFieldInfo(clss, dot->getMember()->getValue(), subtable);

            if (field.isValid()) {
                dot->setSymbol(field.s);

                if (type::getIf<type::ProperType>(field.t) || type::getIf<type::MethodType>(field.t)) {
                    dot->setType(field.t);
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

    if (tuple->getExpressions().size() == 0) {
        tuple->setType(_res.Unit());
    }
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

    if (func == _nextDef && _currentThis) {
        if (isNodeOfType<ClassDecl>(_currentThis, _ctx)) {
            func->setType(_mngr.New<type::MethodType>(static_cast<ClassDecl*>(_currentThis), argTypes, retType));
        } else {
            _rep.fatal(*func, "Unknown type of `this`");
        }
    } else {
        func->setType(_mngr.New<type::FunctionType>(argTypes, retType, nullptr));
    }

    _rep.info(*func->getArgs(), func->type()->toString());
}

void TypeChecking::visit(FunctionCall* call) {
    call->getArgsTuple()->onVisit(this);

    const std::vector<Expression*>& callArgs = call->getArgs();
    std::vector<type::Type*> callArgTypes(callArgs.size());

    for (size_t i = 0; i < callArgs.size(); ++i) {
        callArgTypes[i] = callArgs[i]->type();
    }

    SAVE_MEMBER(_expectedInfo)

    _expectedInfo.args = &callArgTypes;
    _expectedInfo.ret = nullptr;
    _expectedInfo.node = call->getCallee();

    call->getCallee()->onVisit(this);

    RESTORE_MEMBER(_expectedInfo)

    type::Type* calleeT = call->getCallee()->type();

    const std::vector<type::Type*>* expectedArgTypes = nullptr;
    type::Type* retType = nullptr;

    if (type::FunctionType* ft = type::getIf<type::FunctionType>(calleeT)) {
        expectedArgTypes = &ft->getArgTypes();
        retType = ft->getRetType();
    } else if (type::MethodType* mt = type::getIf<type::MethodType>(calleeT)) {
        expectedArgTypes = &mt->getArgTypes();
        retType = mt->getRetType();
    } else {
        _rep.error(*call, "Expression is not callable");
        return;
    }

    if (callArgTypes.size() != expectedArgTypes->size()) {
        _rep.error(*call->getArgsTuple(),
                   "Wrong number of argument. Found " + utils::T_toString(callArgTypes.size()) +
                   ", expected " + utils::T_toString(expectedArgTypes->size()));
        return;
    }

    for (size_t i = 0; i < expectedArgTypes->size(); ++i) {
        if (!callArgTypes[i]->applied(_ctx)->isSubTypeOf((*expectedArgTypes)[i]->applied(_ctx))) {
            _rep.error(*callArgs[i],
                       "Argument type mismatch. Found " + callArgTypes[i]->applied(_ctx)->toString() +
                       ", expected " + (*expectedArgTypes)[i]->applied(_ctx)->toString());
        }
    }

    call->setType(retType);
}

void TypeChecking::visit(Identifier* ident) {
    if (ident->getSymbolCount() == 1) {
        if (sym::Symbol* sym = ident->getSymbol()) {
            if (type::Type* t = tryGetTypeOfSymbol(sym)) {
                ident->setType(t);
            }
        }
    } else if (_expectedInfo.node == ident) {
        FieldInfo info = resolveOverload(*ident, ident->getSymbolDatas().cbegin(), ident->getSymbolDatas().cend(), {});
        ident->setSymbol(info.s);
        ident->setType(info.t);
    } else {
        _rep.error(*ident, "Not enough information are provided to determine the right symbol");
    }
}

void TypeChecking::visit(This* ths) {
    if (!_currentThis) {
        _ctx->reporter().error(*ths, "`this` is forbidden outside of a method scope");
    } else {
        ths->setType(ASTTypeCreator::createType(_currentThis, _ctx));
    }
}

void TypeChecking::visit(BoolLitteral* boollit) {
    boollit->setType(_res.Bool());
}

void TypeChecking::visit(IntLitteral* intlit) {
    intlit->setType(_res.Int());
}

void TypeChecking::visit(RealLitteral* reallit) {
    reallit->setType(_res.Real());
}

TypeChecking::FieldInfo TypeChecking::tryGetFieldInfo(ClassDecl* clss, const std::string& id, const type::SubstitutionTable& subtable) {
    const auto& it = clss->getScope()->getAllSymbols().equal_range(id);

    if (it.first == it.second) {
        return {nullptr, nullptr};
    }

    const sym::SymbolData& data = it.first->second;
    type::Type* tp = type::Type::findSubstitution(subtable, tryGetTypeOfSymbol(data.symbol))->applyEnv(subtable, _ctx);

    return {data.symbol, type::Type::findSubstitution(data.env, tp)->applyEnv(data.env, _ctx)};
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

sym::DefinitionSymbol* TypeChecking::findOverridenSymbol(sym::DefinitionSymbol* def) {
    sym::Scoped* scoped;

    if (isNodeOfType<ClassDecl>(def->getOwner(), _ctx)) {
        scoped = static_cast<ClassDecl*>(def->getOwner());
    } else {
        _rep.fatal(*def, "Owner of " + def->getName() + " is unexpected");
        return nullptr;
    }

    const auto& itPair = scoped->getScope()->getAllSymbols().equal_range(def->getName());

    for (auto it = itPair.first; it != itPair.second; ++it) {
        const sym::SymbolData& data = it->second;

        if (it->second.symbol != def && data.symbol->getSymbolType() == sym::SYM_DEF) {
            sym::DefinitionSymbol* potentialDef = static_cast<sym::DefinitionSymbol*>(data.symbol);
            if (def->type()->isSubTypeOf(potentialDef->type()->applyEnv(data.env, _ctx))) {
                if (potentialDef->getDef()->isRedef()) {
                    if (sym::DefinitionSymbol* alreadyOverriden = potentialDef->getOverridenSymbol()) {
                        return alreadyOverriden;
                    } else {
                        continue; // for readability
                    }
                } else {
                    return potentialDef;
                }
            }
        }
    }

    _rep.error(*def, "Could not find the definition overriden by " +
               def->getName() + " (which has type " + def->type()->toString() + ")");

    return nullptr;
}

template<typename T>
T* applyEnvsHelper(T* t, const type::SubstitutionTable& subtable, const type::SubstitutionTable* env, CompCtx_Ptr& ctx) {
    t = static_cast<T*>(type::Type::findSubstitution(subtable, t)->applyEnv(subtable, ctx));
    t = static_cast<T*>(type::Type::findSubstitution(*env, t)->applyEnv(*env, ctx));
    return t;
}

class OverloadedDefSymbolCandidate final {
public:

    OverloadedDefSymbolCandidate() {}

    size_t argCount() const {
        return _args->size();
    }

    type::Type* arg(size_t index) const {
        return (*_args)[index];
    }

    void incrScore() {
        ++_score;
    }

    uint32_t score() const {
        return _score;
    }

    sym::DefinitionSymbol* symbol() const {
        return _symbol;
    }

    type::Type* appliedType() const {
        return _appliedType;
    }

    static void append(std::vector<OverloadedDefSymbolCandidate>& vec, sym::DefinitionSymbol* s, type::Type* t, size_t expectedArgCount,
                       const type::SubstitutionTable& subtable, const type::SubstitutionTable* env, CompCtx_Ptr& ctx) {
        if (type::FunctionType* ft = type::getIf<type::FunctionType>(t)) {
            if (ft->getArgTypes().size() == expectedArgCount) {
                vec.push_back(OverloadedDefSymbolCandidate(s, applyEnvsHelper(ft, subtable, env, ctx)));
            }
        } else if (type::MethodType* mt = type::getIf<type::MethodType>(t)) {
            if (mt->getArgTypes().size() == expectedArgCount) {
                vec.push_back(OverloadedDefSymbolCandidate(s, applyEnvsHelper(mt, subtable, env, ctx)));
            }
        }
    }

private:

    OverloadedDefSymbolCandidate(sym::DefinitionSymbol* s, type::FunctionType* ft)
        : _symbol(s), _args(&ft->getArgTypes()), _ret(ft->getRetType()), _score(0), _appliedType(ft)
    { }

    OverloadedDefSymbolCandidate(sym::DefinitionSymbol* s, type::MethodType* mt)
        : _symbol(s), _args(&mt->getArgTypes()), _ret(mt->getRetType()), _score(0), _appliedType(mt)
    { }

    sym::DefinitionSymbol* _symbol;
    const std::vector<type::Type*>* _args;
    type::Type* _ret;
    uint32_t _score;

    type::Type* _appliedType;
};

template<typename SymbolIterator>
TypeChecking::FieldInfo TypeChecking::resolveOverload(
        const common::Positionnable& pos,
        const SymbolIterator& begin, const SymbolIterator& end,
        const type::SubstitutionTable& subtable)
{
    std::vector<OverloadedDefSymbolCandidate> candidates;
    size_t expectedArgCount = _expectedInfo.args->size();

    for (SymbolIterator it = begin; it != end; ++it) {
        const sym::Symbolic<sym::Symbol>::SymbolData& data = *it;
        if (data.symbol->getSymbolType() == sym::SYM_DEF) {
            sym::DefinitionSymbol* defsymbol = static_cast<sym::DefinitionSymbol*>(data.symbol);
            OverloadedDefSymbolCandidate::append(candidates, defsymbol, defsymbol->type(), expectedArgCount, subtable, data.env, _ctx);
        }
    }

    size_t candidateCount = candidates.size();

    for (size_t a = 0; a < expectedArgCount; ++a) {
        for (size_t i = 0; i < candidateCount; ++i) {
            type::Type* iType = candidates[i].arg(a);

            for (size_t j = i + 1; j < candidateCount; ++j) {
                type::Type* jType = candidates[j].arg(a);

                if (iType->isSubTypeOf(jType)) { candidates[i].incrScore(); }
                if (jType->isSubTypeOf(iType)) { candidates[j].incrScore(); }
            }
        }
    }

    std::sort(candidates.begin(), candidates.end(), [](const OverloadedDefSymbolCandidate& a, const OverloadedDefSymbolCandidate& b) {
        return a.score() > b.score();
    });

    for (const OverloadedDefSymbolCandidate& candidate : candidates) {
        std::string args = "[Candidate] score=" + utils::T_toString(candidate.score()) + " (";
        if (candidate.argCount() > 0) {
            for (size_t i = 0; i < candidate.argCount() - 1; ++i) {
                args += candidate.arg(i)->toString() + ", ";
            }
            args += candidate.arg(candidate.argCount() - 1)->toString();
        }
        args += ")";

        _rep.info(*candidate.symbol(), args);
    }

    std::vector<OverloadedDefSymbolCandidate*> theChosenOnes;

    for (OverloadedDefSymbolCandidate& candidate : candidates) {
        bool ok = true;

        for (size_t a = 0; a < expectedArgCount; ++a) {
            if (!((*_expectedInfo.args)[a]->isSubTypeOf(candidate.arg(a)))) {
                ok = false;
                break;
            }
        }

        if (ok && (theChosenOnes.size() > 0) && (theChosenOnes[0]->score() > candidate.score())) {
           break;
        } else if (ok) {
           theChosenOnes.push_back(&candidate);
        }
    }

    switch (theChosenOnes.size()) {
    case 0:
        _rep.error(pos, "No viable candidate found among " + utils::T_toString(candidates.size()) + " overloads");
        return FieldInfo(nullptr, type::Type::NotYetDefined());

    default:
        _rep.error(pos, "Ambiguous symbol access. Multiple candidates match the required type:");
        for (OverloadedDefSymbolCandidate* candidate : theChosenOnes) {
            _rep.info(*candidate->symbol(), "Is a viable candidate (has type " + candidate->appliedType()->toString() + ")");
        }

    case 1:
        return FieldInfo(theChosenOnes[0]->symbol(), theChosenOnes[0]->appliedType());
    }
}

// FIELD INFO

TypeChecking::FieldInfo::FieldInfo(sym::Symbol* sy, type::Type* ty) : s(sy), t(ty) {

}

bool TypeChecking::FieldInfo::isValid() const {
    return s && t;
}

}

}
