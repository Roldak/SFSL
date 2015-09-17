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
#include "../Symbols/Scope.h"

#include "../../../Utils/TakeSecondIterator.h"

#define DEBUG_FUNCTION_OVERLOADING

namespace sfsl {

namespace ast {

// HELPER

template<typename T>
T* applyEnvsHelper(T* t, const type::SubstitutionTable& subtable, const type::SubstitutionTable* env, CompCtx_Ptr& ctx) {
    type::Type* tmpT = t;
    type::SubstitutionTable unionTable = subtable;

    if (env) {
        unionTable.insert(env->begin(), env->end());
    }

    tmpT = type::Type::findSubstitution(unionTable, tmpT)->substitute(unionTable, ctx)->apply(ctx);

    return static_cast<T*>(tmpT);
}

// TYPE CHECKER

TypeChecker::TypeChecker(CompCtx_Ptr& ctx, const sym::SymbolResolver& res) : ASTImplicitVisitor(ctx), _res(res), _rep(ctx->reporter()) {

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
    ASTImplicitVisitor::visit(clss);

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

    ASTImplicitVisitor::visit(func);
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
    ASTImplicitVisitor::visit(prog);

    for (DefineDecl* redef : _redefs) {
        redef->getSymbol()->setOverridenSymbol(findOverridenSymbol(redef->getSymbol()));
    }
}

void TypeChecking::visit(TypeDecl* tdecl) {
    tdecl->setType(_res.Unit());

    ASTImplicitVisitor::visit(tdecl);
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
    ASTImplicitVisitor::visit(exp);
    exp->setType(exp->getExpression()->type());
}

void TypeChecking::visit(BinaryExpression* bin) {
    ASTImplicitVisitor::visit(bin);
    bin->setType(bin->getLhs()->type()); // TODO : make it right
}

void TypeChecking::visit(AssignmentExpression* aex) {
    ASTImplicitVisitor::visit(aex);

    if (!ASTAssignmentChecker::isExpressionAssignable(aex->getLhs(), _ctx)) {
        _rep.error(*aex, "Left hand side is not an assignable expression");
    }

    type::Type* lhsT = aex->getLhs()->type();
    type::Type* rhsT = aex->getRhs()->type();

    if (type::TypeToBeInferred* tbi = type::getIf<type::TypeToBeInferred>(lhsT)) {
        tbi->assignInferredType(rhsT);
    } else if (!rhsT->apply(_ctx)->isSubTypeOf(lhsT->apply(_ctx))) {
        _rep.error(*aex, "Assigning incompatible type. Found " +
                   rhsT->toString() + ", expected " + lhsT->toString());
    }

    aex->setType(rhsT);
}

void TypeChecking::visit(TypeSpecifier* tps) {
    ASTImplicitVisitor::visit(tps);

    Identifier* id = tps->getSpecified();
    type::Typed* tped = nullptr;

    if (id->getSymbol()->getSymbolType() == sym::SYM_VAR) {
        tped = static_cast<sym::VariableSymbol*>(id->getSymbol());
    } else if (id->getSymbol()->getSymbolType() == sym::SYM_DEF) {
        tped = static_cast<sym::DefinitionSymbol*>(id->getSymbol()); // for later
    }

    if (isNodeOfType<TypeToBeInferred>(tps->getTypeNode(), _ctx)) {
        type::TypeToBeInferred* t = type::TypeToBeInferred::create({tped, tps}, _ctx);
        tped->setType(t);
        tps->setType(t);
    } else if (type::Type* tpe = ASTTypeCreator::createType(tps->getTypeNode(), _ctx)) {
        tped->setType(tpe);
        tps->setType(tpe);
    } else {
        _ctx->reporter().error(*tps->getTypeNode(), "Expression is not a type");
    }
}

void TypeChecking::visit(Block* block) {
    ASTImplicitVisitor::visit(block);

    const std::vector<Expression*>& stats = block->getStatements();
    if (stats.size() > 0) {
        block->setType(stats.back()->type());
    } else {
        block->setType(_res.Unit());
    }
}

void TypeChecking::visit(IfExpression* ifexpr) {
    ASTImplicitVisitor::visit(ifexpr);

    if (!ifexpr->getCondition()->type()->apply(_ctx)->isSubTypeOf(_res.Bool())) {
        _rep.error(*ifexpr->getCondition(), "Condition is not a boolean (Found " + ifexpr->getCondition()->type()->toString() + ")");
    }

    type::Type* thenType = ifexpr->getThen()->type();

    if (ifexpr->getElse()) {
        type::Type* elseType = ifexpr->getElse()->type();

        if (thenType->apply(_ctx)->isSubTypeOf(elseType->apply(_ctx))) {
            ifexpr->setType(elseType);
        } else if (elseType->apply(_ctx)->isSubTypeOf(thenType->apply(_ctx))) {
            ifexpr->setType(thenType);
        } else {
            _rep.error(*ifexpr, "The then-part and else-part have different types. Found " +
                       thenType->toString() + " and " + elseType->toString());
        }
        std::cout << "LASLDASLKDADS" << std::endl;

    } else {
        if (!thenType->apply(_ctx)->isSubTypeOf(_res.Unit())) {
            _rep.error(*ifexpr->getThen(), "An if-expression without else-part must have its then-part evaluate to unit. Found " +
                       thenType->toString());
        }
        ifexpr->setType(_res.Unit());
    }
}

void TypeChecking::visit(MemberAccess* dot) {
    dot->getAccessed()->onVisit(this);

    if (type::Type* t = dot->getAccessed()->type()) {
        if (type::ProperType* obj = type::getIf<type::ProperType>(t->apply(_ctx))) {
            ClassDecl* clss = obj->getClass();
            const type::SubstitutionTable& subtable = obj->getSubstitutionTable();

            FieldInfo field = tryGetFieldInfo(dot, clss, dot->getMember()->getValue(), subtable);

            if (field.isValid()) {
                dot->setSymbol(field.s);

                if (type::getIf<type::ProperType>(field.t->apply(_ctx))
                        || type::getIf<type::MethodType>(field.t->apply(_ctx))) {
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
    ASTImplicitVisitor::visit(tuple);

    if (tuple->getExpressions().size() == 0) {
        tuple->setType(_res.Unit());
    }
}

void TypeChecking::visit(FunctionCreation* func) {
    ASTImplicitVisitor::visit(func);

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

    type::Type* calleeT = call->getCallee()->type()->applyTCCallsOnly(_ctx);

    const std::vector<type::Type*>* expectedArgTypes = nullptr;
    type::Type* retType = nullptr;

    if (type::FunctionType* ft = type::getIf<type::FunctionType>(calleeT)) {
        expectedArgTypes = &static_cast<type::FunctionType*>(ft->apply(_ctx))->getArgTypes();
        retType = ft->getRetType();
    } else if (type::MethodType* mt = type::getIf<type::MethodType>(calleeT)) {
        expectedArgTypes = &static_cast<type::FunctionType*>(calleeT->apply(_ctx))->getArgTypes();
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
        if (!callArgTypes[i]->apply(_ctx)->isSubTypeOf((*expectedArgTypes)[i]->apply(_ctx))) {
            _rep.error(*callArgs[i],
                       "Argument type mismatch. Found " + callArgTypes[i]->apply(_ctx)->toString() +
                       ", expected " + (*expectedArgTypes)[i]->apply(_ctx)->toString());
        }
    }

    call->setType(retType);
}

void TypeChecking::visit(Identifier* ident) {
    const AnySymbolicData data = resolveOverload(ident, ident->getSymbolDatas().cbegin(), ident->getSymbolDatas().cend(), {});
    if (sym::Symbol* sym = data.symbol) {
        ident->setSymbol(sym);
        if (type::Type* t = tryGetTypeOfSymbol(sym)) {
            ident->setType(data.env ? type::Type::findSubstitution(*data.env, t)->substitute(*data.env, _ctx) : t);
        }
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

void TypeChecking::visit(StringLitteral* strlit) {
    strlit->setType(_res.String());
}

TypeChecking::FieldInfo TypeChecking::tryGetFieldInfo(MemberAccess* dot, ClassDecl* clss, const std::string& id, const type::SubstitutionTable& subtable) {
    const auto& it = clss->getScope()->getAllSymbols().equal_range(id);

    if (it.first == it.second) {
        return {nullptr, nullptr};
    }

    auto b = utils::TakeSecondIterator<decltype(it.first)>(it.first);
    auto e = utils::TakeSecondIterator<decltype(it.second)>(it.second);
    const AnySymbolicData data = resolveOverload(dot, b, e, subtable);

    if (data.symbol) {
        type::Type* t = tryGetTypeOfSymbol(data.symbol);
        if (data.env) {
            t = type::Type::findSubstitution(*data.env, t)->substitute(*data.env, _ctx);
        }
        t = type::Type::findSubstitution(subtable, t)->substitute(subtable, _ctx);

        return {data.symbol, t};
    } else {
        return {nullptr, nullptr};
    }
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
    type::Type* defType = def->type()->apply(_ctx);

    for (auto it = itPair.first; it != itPair.second; ++it) {
        const sym::SymbolData& data = it->second;

        if (data.symbol != def && data.symbol->getSymbolType() == sym::SYM_DEF) {
            sym::DefinitionSymbol* potentialDef = static_cast<sym::DefinitionSymbol*>(data.symbol);

            if (defType->isSubTypeOf(potentialDef->type()->substitute(data.env, _ctx)->apply(_ctx))) {
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

class OverloadedDefSymbolCandidate final {
public:

    OverloadedDefSymbolCandidate() {}

    sym::DefinitionSymbol* symbol() const {
        return _symbol;
    }

    const type::SubstitutionTable* env() const {
        return _env;
    }

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

    type::Type* appliedType() const {
        return _appliedType;
    }

    static void append(std::vector<OverloadedDefSymbolCandidate>& vec, sym::DefinitionSymbol* s, type::Type* t, size_t expectedArgCount,
                       const type::SubstitutionTable& subtable, const type::SubstitutionTable* env, CompCtx_Ptr& ctx)
    {
        if (type::FunctionType* ft = type::getIf<type::FunctionType>(t)) {
            if (ft->getArgTypes().size() == expectedArgCount) {
                vec.push_back(OverloadedDefSymbolCandidate(s, env, applyEnvsHelper(ft, subtable, env, ctx)));
            }
        } else if (type::MethodType* mt = type::getIf<type::MethodType>(t)) {
            if (mt->getArgTypes().size() == expectedArgCount) {
                vec.push_back(OverloadedDefSymbolCandidate(s, env, applyEnvsHelper(mt, subtable, env, ctx)));
            }
        }
    }

private:

    OverloadedDefSymbolCandidate(sym::DefinitionSymbol* s, const type::SubstitutionTable* env, type::FunctionType* ft)
        : _symbol(s), _env(env), _args(&ft->getArgTypes()), _ret(ft->getRetType()), _score(0), _appliedType(ft)
    { }

    OverloadedDefSymbolCandidate(sym::DefinitionSymbol* s, const type::SubstitutionTable* env, type::MethodType* mt)
        : _symbol(s), _env(env), _args(&mt->getArgTypes()), _ret(mt->getRetType()), _score(0), _appliedType(mt)
    { }

    sym::DefinitionSymbol* _symbol;
    const type::SubstitutionTable* _env;

    const std::vector<type::Type*>* _args;
    type::Type* _ret;
    uint32_t _score;

    type::Type* _appliedType;
};

void debugDumpCandidateScores(const std::vector<OverloadedDefSymbolCandidate>& candidates, CompCtx_Ptr& ctx) {
    for (const OverloadedDefSymbolCandidate& candidate : candidates) {
        std::string args = "[Candidate] score=" + utils::T_toString(candidate.score()) + " (";
        if (candidate.argCount() > 0) {
            for (size_t i = 0; i < candidate.argCount() - 1; ++i) {
                args += candidate.arg(i)->toString() + ", ";
            }
            args += candidate.arg(candidate.argCount() - 1)->toString();
        }
        args += ")";

        ctx->reporter().info(*candidate.symbol(), args);
    }
}

template<typename SymbolIterator>
TypeChecking::AnySymbolicData TypeChecking::resolveOverload(
        ASTNode* triggerer,
        const SymbolIterator& begin, const SymbolIterator& end,
        const type::SubstitutionTable& subtable)
{
    if (std::distance(begin, end) == 1) {
        const auto& val = *begin;
        return AnySymbolicData(val.symbol, val.env);
    } else if (_expectedInfo.node != triggerer) {
        _rep.error(*triggerer, "Not enough information are provided to determine the right symbol");
        return {nullptr, nullptr};
    }

    std::vector<OverloadedDefSymbolCandidate> candidates;
    size_t expectedArgCount = _expectedInfo.args->size();

    for (SymbolIterator it = begin; it != end; ++it) {
        const auto& val = *it;
        const AnySymbolicData data(val.symbol, val.env);
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

#ifdef DEBUG_FUNCTION_OVERLOADING
    debugDumpCandidateScores(candidates, _ctx);
#endif

    std::vector<OverloadedDefSymbolCandidate*> theChosenOnes;

    for (OverloadedDefSymbolCandidate& candidate : candidates) {
        bool matches = true;

        for (size_t a = 0; a < expectedArgCount; ++a) {
            if (!((*_expectedInfo.args)[a]->isSubTypeOf(candidate.arg(a)))) {
                matches = false;
                break;
            }
        }

        if ( matches &&
             theChosenOnes.size() > 0 &&
             theChosenOnes[0]->score() > candidate.score()) {
           break;
        } else if (matches) {
           theChosenOnes.push_back(&candidate);
        }
    }

    switch (theChosenOnes.size()) {
    case 0:
        _rep.error(*triggerer, "No viable candidate found among " + utils::T_toString(candidates.size()) + " overloads");
        return {nullptr, nullptr};

    default: {
        size_t properDefCount = 0;
        for (OverloadedDefSymbolCandidate* candidate : theChosenOnes) {
            if (!candidate->symbol()->getDef()->isRedef()) {
                theChosenOnes[0] = candidate;
                ++properDefCount;
            }
        }

        if (properDefCount > 1) {
            _rep.error(*triggerer, "Ambiguous symbol access. Multiple candidates match the required type:");
            for (OverloadedDefSymbolCandidate* candidate : theChosenOnes) {
                _rep.info(*candidate->symbol(), "Is a viable candidate (has type " + candidate->appliedType()->toString() + ")");
            }
        }
    }

    case 1:
        return {theChosenOnes[0]->symbol(), theChosenOnes[0]->env()};
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
