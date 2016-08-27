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
#include "../AST/Visitors/ASTSymbolExtractor.h"
#include "../AST/Visitors/ASTAssignmentChecker.h"
#include "../Symbols/Scope.h"

#include "../../../Utils/TakeSecondIterator.h"

//#define DEBUG_FUNCTION_OVERLOADING

namespace sfsl {

namespace ast {

// HELPER

template<typename T>
T* applyEnvsHelper(T* t, const type::Environment& env, const type::Environment* dataEnv, CompCtx_Ptr& ctx) {
    type::Type* tmpT = t;
    type::Environment unionEnv = env;

    if (dataEnv) {
        unionEnv.insert(dataEnv->begin(), dataEnv->end());
    }

    tmpT = tmpT->substitute(unionEnv, ctx)->apply(ctx);

    return static_cast<T*>(tmpT);
}

// TYPE CHECKER

TypeChecker::TypeChecker(CompCtx_Ptr& ctx, const common::AbstractPrimitiveNamer& namer, const sym::SymbolResolver& res)
    : ASTImplicitVisitor(ctx), _namer(namer), _res(res), _rep(ctx->reporter()) {

}

TypeChecker::~TypeChecker() {

}

// TOP LEVEL TYPE CHECKING

TopLevelTypeChecking::TopLevelTypeChecking(CompCtx_Ptr& ctx, const common::AbstractPrimitiveNamer& namer, const sym::SymbolResolver& res)
    : TypeChecker(ctx, namer, res) {

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

void TopLevelTypeChecking::visit(DefineDecl* decl) {
    if (TypeExpression* typeExpr = decl->getTypeSpecifier()) {
        typeExpr->onVisit(this);
        type::Type* tp = ASTTypeCreator::createType(typeExpr, _ctx);
        decl->getSymbol()->setType(tp);

        // visit value but don't need to create the type, so don't set _nextDef
        if (Expression* value = decl->getValue()) {
            value->onVisit(this);
        }
    } else if (Expression* value = decl->getValue()) {
        _nextDef = value;
        value->onVisit(this);
        decl->getSymbol()->setType(value->type());
    }
}

void TopLevelTypeChecking::visit(FunctionCreation* func) {
    if (func == _nextDef) {
        // to be able to resolve overloads without having to fully typecheck the body of the function

        Expression* expr = func->getArgs();
        std::vector<Expression*> args;

        if (Tuple* tuple = getIfNodeOfType<Tuple>(expr, _ctx)) { // form is `() => ...` or `(exp, exp) => ...`, ...
            args = tuple->getExpressions();
        } else { // form is `exp => ...` or `(exp) => ...`
            args.push_back(expr);
        }

        std::vector<type::Type*> argTypes(args.size());

        for (size_t i = 0; i < args.size(); ++i) {
            if (TypeSpecifier* tps = getIfNodeOfType<TypeSpecifier>(args[i], _ctx)) {
                argTypes[i] = ASTTypeCreator::createType(tps->getTypeNode(), _ctx);
            } else {
                _rep.error(*args[i], "Omitting the type of the argument is forbidden in this case");
                argTypes[i] = type::Type::NotYetDefined();
            }
        }

        std::vector<TypeExpression*> typeArgs;
        if (func->getTypeArgs()) {
            typeArgs = func->getTypeArgs()->getExpressions();
        }
        func->setType(_mngr.New<type::FunctionType>(typeArgs, argTypes, type::Type::NotYetDefined(), nullptr, type::Environment::Empty));
    }

    ASTImplicitVisitor::visit(func);
}

// TYPE CHECKING

TypeChecking::TypeChecking(CompCtx_Ptr& ctx, const common::AbstractPrimitiveNamer& namer, const sym::SymbolResolver& res)
    : TypeChecker(ctx, namer, res), _currentThis(nullptr), _nextDef(nullptr), _triggeringDef(nullptr), _expectedInfo{nullptr, nullptr, nullptr, nullptr} {

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

void TypeChecking::visit(ClassDecl* clss) {
    ASTImplicitVisitor::visit(clss);

    if (clss->isAbstract()) {
        return;
    }

    int abstractOverRedef = 0;

    for (const std::pair<std::string, sym::SymbolData>& pair : clss->getScope()->getAllSymbols()) {
        if (sym::DefinitionSymbol* defsym = sym::getIfSymbolOfType<sym::DefinitionSymbol>(pair.second.symbol)) {
            if (defsym->getDef()->isAbstract()) {
                ++abstractOverRedef;
            } else if (defsym->getDef()->isRedef()) {
                --abstractOverRedef;
            }
        }
    }

    if (abstractOverRedef > 0) {
        _rep.error(*clss, "Non abstract class must redefine every one of its abstract members");
    }
}

void TypeChecking::visit(DefineDecl* decl) {
    if (TRY_INSERT(_visitedDefs, decl)) {
        decl->setType(_res.Unit());

        SAVE_MEMBER_AND_SET(_currentThis, decl->getSymbol()->getOwner())
        SAVE_MEMBER_AND_SET(_nextDef, decl->getValue())
        SAVE_MEMBER_AND_SET(_triggeringDef, decl->getSymbol());

        type::Type* expectedType = nullptr;
        type::Type* foundType = nullptr;

        SAVE_MEMBER(_expectedInfo)

        if (TypeExpression* tpexpr = decl->getTypeSpecifier()) {
            tpexpr->onVisit(this);
            expectedType = ASTTypeCreator::createType(tpexpr, _ctx);

            if (expectedType->applyTCCallsOnly(_ctx)->getTypeKind() == type::TYPE_FUNCTION && _currentThis) {
                expectedType = type::MethodType::fromFunctionType(
                            static_cast<type::FunctionType*>(expectedType->applyTCCallsOnly(_ctx)),
                            static_cast<ast::ClassDecl*>(_currentThis), _ctx);
            }

            _expectedInfo.node = decl->getValue();
            _expectedInfo.ret = expectedType;
        }

        if (Expression* value = decl->getValue()) {
            value->onVisit(this);
            foundType = value->type();
        }

        RESTORE_MEMBER(_expectedInfo)

        RESTORE_MEMBER(_triggeringDef)
        RESTORE_MEMBER(_nextDef)
        RESTORE_MEMBER(_currentThis)

        if (foundType) {
            type::Type* appliedFT = foundType->applyTCCallsOnly(_ctx);
            if (!type::getIf<type::ProperType>(appliedFT) && !type::getIf<type::MethodType>(appliedFT)) {
                _rep.error(*decl->getValue(), "Right-hand side of definition must evaluate to a proper type. Found " + foundType->apply(_ctx)->toString());
            }
        }

        if (expectedType && foundType) {
            if (!foundType->apply(_ctx)->isSubTypeOf(expectedType->apply(_ctx), _ctx)) {
                _rep.error(*decl->getValue(),
                           "Type mismatch. Expected " + expectedType->apply(_ctx)->toString(&_ctx) +
                           ", found " + foundType->apply(_ctx)->toString(&_ctx));
            }

            decl->getName()->setType(expectedType);
            decl->getSymbol()->setType(expectedType);
        }
        else if (expectedType) {
            decl->getName()->setType(expectedType);
            decl->getSymbol()->setType(expectedType);
        }
        else if (foundType) {
            decl->getName()->setType(foundType);
            decl->getSymbol()->setType(foundType);
        }
        else { // "cannot happen"
            _rep.fatal(*decl, "Define statement cannot be typechecked");
        }

        if (decl->isRedef()) {
            _redefs.push_back(decl);
        }
    }
}

void TypeChecking::visit(ExpressionStatement* exp) {
    ASTImplicitVisitor::visit(exp);
    exp->setType(exp->getExpression()->type());
}

void TypeChecking::visit(AssignmentExpression* aex) {
    aex->getLhs()->onVisit(this);

    SAVE_MEMBER(_expectedInfo)
    _expectedInfo.node = aex->getRhs();
    _expectedInfo.ret = aex->getLhs()->type();

    aex->getRhs()->onVisit(this);

    RESTORE_MEMBER(_expectedInfo)

    type::Type* lhsT = aex->getLhs()->type();
    type::Type* rhsT = aex->getRhs()->type();

    if (rhsT->apply(_ctx)->getTypeKind() == type::TYPE_NYD) {
        _rep.error(*aex, "Right hand side does not have any type");
    } else if (type::TypeToBeInferred* tbi = type::getIf<type::TypeToBeInferred>(lhsT)) {
        tbi->assignInferredType(rhsT);
    } else if (!rhsT->apply(_ctx)->isSubTypeOf(lhsT->apply(_ctx), _ctx)) {
        _rep.error(*aex, "Assigning incompatible type. Expected " +
                   lhsT->apply(_ctx)->toString(&_ctx) + ", found " + rhsT->apply(_ctx)->toString(&_ctx));
    }

    aex->setType(rhsT);
}

void TypeChecking::visit(TypeSpecifier* tps) {
    ASTImplicitVisitor::visit(tps);

    Identifier* id = tps->getSpecified();
    type::Typed* tped = nullptr;

    if (sym::VariableSymbol* varsym = sym::getIfSymbolOfType<sym::VariableSymbol>(id->getSymbol())) {
        tped = varsym;
    } else if (sym::DefinitionSymbol* defsym = sym::getIfSymbolOfType<sym::DefinitionSymbol>(id->getSymbol())) {
        tped = defsym;
    }

    if (isNodeOfType<TypeToBeInferred>(tps->getTypeNode(), _ctx)) {
        type::TypeToBeInferred* t = type::TypeToBeInferred::create({tped, tps}, _ctx);
        tped->setType(t);
        tps->setType(t);
    } else if (type::Type* tpe = ASTTypeCreator::createType(tps->getTypeNode(), _ctx)) {
        tped->setType(tpe);
        tps->setType(tpe);
    } else {
        _rep.error(*tps->getTypeNode(), "Expression is not a type");
    }
}

void TypeChecking::visit(Block* block) {
    const std::vector<Expression*>& stats = block->getStatements();

    if (stats.size() > 0) {
        SAVE_MEMBER(_expectedInfo)
        _expectedInfo.ret = _res.Unit();

        for (size_t i = 0; i < stats.size() - 1; ++i) {
            _expectedInfo.node = stats[i];
            stats[i]->onVisit(this);
        }

        RESTORE_MEMBER(_expectedInfo)

        _expectedInfo.node = stats.back();
        stats.back()->onVisit(this);

        RESTORE_MEMBER(_expectedInfo)

        if (_expectedInfo.node == block && _expectedInfo.ret && _expectedInfo.ret->isSubTypeOf(_res.Unit(), _ctx)) {
            block->setType(_res.Unit());
        } else {
            block->setType(stats.back()->type());
        }
    } else {
        block->setType(_res.Unit());
    }
}

void TypeChecking::visit(IfExpression* ifexpr) {
    ifexpr->getCondition()->onVisit(this);

    if (!ifexpr->getCondition()->type()->apply(_ctx)->isSubTypeOf(_res.Bool(), _ctx)) {
        if (ifexpr->isFromLazyOperator()) {
            _rep.error(*ifexpr->getCondition(), "Operands of the `&&` and `||` operators must be booleans");
        } else {
            _rep.error(*ifexpr->getCondition(), "Condition is not a boolean (Found " + ifexpr->getCondition()->type()->apply(_ctx)->toString() + ")");
        }
    }

    if (_expectedInfo.node == ifexpr && _expectedInfo.ret && _expectedInfo.ret->isSubTypeOf(_res.Unit(), _ctx)) {
        ifexpr->setType(_res.Unit());

        _expectedInfo.node = ifexpr->getThen();
        ifexpr->getThen()->onVisit(this);

        if (Expression* expr = ifexpr->getElse()) {
            _expectedInfo.node = ifexpr->getElse();
            expr->onVisit(this);
        }
        _expectedInfo.node = ifexpr;

        return;
    }

    ifexpr->getThen()->onVisit(this);
    type::Type* thenType = ifexpr->getThen()->type();

    if (ifexpr->getElse()) {
        ifexpr->getElse()->onVisit(this);
        type::Type* elseType = ifexpr->getElse()->type();

        if (thenType->apply(_ctx)->isSubTypeOf(elseType->apply(_ctx), _ctx)) {
            ifexpr->setType(elseType);
        } else if (elseType->apply(_ctx)->isSubTypeOf(thenType->apply(_ctx), _ctx)) {
            ifexpr->setType(thenType);
        } else if (ifexpr->isFromLazyOperator()) {
            // then and else parts have the same position in this case
            _rep.error(*ifexpr->getThen(), "Operands of the `&&` and `||` operators must be booleans");
        } else {
            _rep.error(*ifexpr, "The then-part and else-part have different types. Found " +
                       thenType->apply(_ctx)->toString(&_ctx) + " and " + elseType->apply(_ctx)->toString(&_ctx));
        }
    } else {
        if (!thenType->apply(_ctx)->isSubTypeOf(_res.Unit(), _ctx)) {
            _rep.error(*ifexpr->getThen(), "An if-expression without else-part must have its then-part evaluate to unit. Found " +
                       thenType->apply(_ctx)->toString());
        }
        ifexpr->setType(_res.Unit());
    }
}

void TypeChecking::visit(MemberAccess* dot) {
    dot->getAccessed()->onVisit(this);

    if (type::Type* t = dot->getAccessed()->type()) {
        if (type::ProperType* obj = type::getIf<type::ProperType>(t->apply(_ctx))) {
            ClassDecl* clss = obj->getClass();
            const type::Environment& env = obj->getEnvironment();

            FieldInfo field = tryGetFieldInfo(dot, clss, dot->getMember()->getValue(), env);

            if (field.isValid()) {
                dot->setSymbol(field.s);

                if (type::getIf<type::ProperType>(field.t->apply(_ctx))
                        || type::getIf<type::MethodType>(field.t->apply(_ctx))) {
                    dot->setType(field.t);
                } else {
                    _rep.error(*dot->getMember(), "Member `" + dot->getMember()->getValue() +
                               "` of class " + clss->getName() + " is not a value");
                }
            } else {
                _rep.error(*dot->getMember(), "No member named `" + dot->getMember()->getValue() +
                           "` in class " + clss->getName());
            }

            return;
        }
    }

    tryAssigningTypeToSymbolic(dot);
}

void TypeChecking::visit(Tuple* tuple) {
    ASTImplicitVisitor::visit(tuple);

    if (tuple->getExpressions().size() == 0) {
        tuple->setType(_res.Unit());
    }
}

void TypeChecking::visit(FunctionCreation* func) {
    func->getArgs()->onVisit(this);

    Expression* expr = func->getArgs();
    std::vector<Expression*> args;

    if (Tuple* tuple = getIfNodeOfType<Tuple>(expr, _ctx)) { // form is `() => ...` or `(exp, exp) => ...`, ...
        args = tuple->getExpressions();
    } else { // form is `exp => ...` or `(exp) => ...`
        args.push_back(expr);
    }

    std::vector<type::Type*> argTypes(args.size());
    type::Type* retType = nullptr;

    bool isIncomplete = false;

    for (size_t i = 0; i < args.size(); ++i) {
        argTypes[i] = args[i]->type();
        isIncomplete |= argTypes[i]->getTypeKind() == type::TYPE_TBI;
    }

    if (isIncomplete && _expectedInfo.node == func && _expectedInfo.ret) {
        if (type::ValueConstructorType* expectedValueConstructor = type::getIf<type::ValueConstructorType>(_expectedInfo.ret->applyTCCallsOnly(_ctx))) {
            std::vector<sym::VariableSymbol*> params(ASTAssignmentChecker::getAssignedVars(func->getArgs(), _ctx));

            if (params.size() == args.size() && expectedValueConstructor->getArgTypes().size() == args.size()) {
                for (size_t i = 0; i < params.size(); ++i) {
                    if (type::TypeToBeInferred* tbi = type::getIf<type::TypeToBeInferred>(params[i]->type())) {
                        tbi->assignInferredType(expectedValueConstructor->getArgTypes()[i]);
                        argTypes[i] = params[i]->type();
                    }
                }

                isIncomplete = false;
            }
        }
    }

    if (isIncomplete) {
        _rep.error(*func->getArgs(), "Some of the function's parameters are not annotated with a type and not enough information are available to infer them");
        func->setType(type::Type::NotYetDefined());
        return;
    }

    if (TypeExpression* retTypeExpr = func->getReturnType()) {
        func->getReturnType()->onVisit(this);
        if (!(retType = ASTTypeCreator::createType(retTypeExpr, _ctx))) {
            _rep.error(*retTypeExpr, "Invalid return type");
        }
    } else {
        _triggeringDef->setType(type::Type::NotYetDefined()); // in case of recursive call
        func->getBody()->onVisit(this);
        retType = func->getBody()->type();
    }

    if (func == _nextDef && _currentThis) {
        // func is a method

        if (ClassDecl* clss = getIfNodeOfType<ClassDecl>(_currentThis, _ctx)) {
            func->setType(_mngr.New<type::MethodType>(clss,
                                                      func->getTypeArgs() ? func->getTypeArgs()->getExpressions() : std::vector<TypeExpression*>(),
                                                      argTypes, retType, ASTTypeCreator::buildEnvironmentFromTypeParametrizable(func)));
        } else {
            _rep.fatal(*func, "Unknown type of `this`");
        }
    } else {
        // func is a free function
        assignFunctionType(func, argTypes, retType);
    }

    if (func->getReturnType()) {
        _triggeringDef->setType(func->type());

        SAVE_MEMBER(_expectedInfo)
        _expectedInfo.ret = retType;
        _expectedInfo.node = func->getBody();

        func->getBody()->onVisit(this);

        RESTORE_MEMBER(_expectedInfo)

        if (!func->getBody()->type()->apply(_ctx)->isSubTypeOf(retType->apply(_ctx), _ctx)) {
            _rep.error(*func->getBody(),
                       "Return type mismatch. Expected " + retType->apply(_ctx)->toString(&_ctx) +
                       ", found " + func->getBody()->type()->apply(_ctx)->toString(&_ctx));
        }
    }
}

void TypeChecking::visit(FunctionCall* call) {
    std::vector<TypeExpression*> callTypeArgs = call->getTypeArgsTuple() ? call->getTypeArgs() : std::vector<TypeExpression*>();

    const std::vector<Expression*>& callArgs = call->getArgs();
    ArgTypeEvaluator argTypes(this, callArgs);

    SAVE_MEMBER(_expectedInfo)

    _expectedInfo.typeArgs = &callTypeArgs;
    _expectedInfo.args = &argTypes;
    _expectedInfo.ret = nullptr;
    _expectedInfo.node = call->getCallee();

    call->getCallee()->onVisit(this);

    type::Type* calleeT = call->getCallee()->type()->applyTCCallsOnly(_ctx);

    const std::vector<type::Type*>* expectedArgTypes = nullptr;
    type::Type* retType = nullptr;

    if (Instantiation* inst = getIfNodeOfType<Instantiation>(call->getCallee(), _ctx)) {
        _expectedInfo.node = inst;

        if (type::ProperType* pt = type::getIf<type::ProperType>(calleeT)) {
            if (!transformIntoCallToMember(call, inst, pt, "new", callTypeArgs, &argTypes, expectedArgTypes, retType)) {
                call->setType(inst->type());
                return;
            }
        } else {
            // an error should already have been reported by the typechecking of the Instantiation node
            return;
        }

        retType = inst->type(); // force constructor to return type of its `this`
    } else if (type::MethodType* mt = type::getIf<type::MethodType>(
                   ASTTypeCreator::evalFunctionConstructor(calleeT, callTypeArgs, *call, _ctx, &argTypes))) {

        expectedArgTypes = &mt->getArgTypes();
        retType = mt->getRetType();
    } else if (type::ProperType* pt = type::getIf<type::ProperType>(calleeT)) {
        if (!transformIntoCallToMember(call, call->getCallee(), pt, "()", callTypeArgs, &argTypes, expectedArgTypes, retType)) {
            return;
        }
    } else {
        _rep.error(*call, "Expression is not callable");
        return;
    }

    argTypes.evalAll(*expectedArgTypes);
    call->setType(retType);

    RESTORE_MEMBER(_expectedInfo)

    if (argTypes.size() != expectedArgTypes->size()) {
        _rep.error(*call->getArgsTuple(),
                   "Wrong number of argument. Found " + utils::T_toString(argTypes.size()) +
                   ", expected " + utils::T_toString(expectedArgTypes->size()));
        return;
    }

    for (size_t i = 0; i < expectedArgTypes->size(); ++i) {
        if (!argTypes.at(i)->apply(_ctx)->isSubTypeOf(expectedArgTypes->at(i)->apply(_ctx), _ctx)) {
            _rep.error(*callArgs[i],
                       "Argument type mismatch. Found " + argTypes.at(i)->apply(_ctx)->toString(&_ctx) +
                       ", expected " + expectedArgTypes->at(i)->apply(_ctx)->toString(&_ctx));
        }
    }
}

void TypeChecking::visit(Instantiation* inst) {
    type::Type* instType = ASTTypeCreator::createType(inst->getInstantiatedExpression(), _ctx);
    if (type::ProperType* pt = type::getIf<type::ProperType>(instType->applyTCCallsOnly(_ctx))) {
        if (pt->getClass()->isAbstract()) {
            _rep.error(*inst, "Cannot instantiate abstract class " + pt->getClass()->getName());
        }
        inst->setType(instType);
    } else {
        _rep.error(*inst, "Only classes can be instantiated. Found " + instType->apply(_ctx)->toString());
    }
}

void TypeChecking::visit(Identifier* ident) {
    tryAssigningTypeToSymbolic(ident);
}

void TypeChecking::visit(This* ths) {
    if (!_currentThis) {
        _rep.error(*ths, "`this` is forbidden outside of a method scope");
    } else {
        ths->setType(ASTTypeCreator::createType(_currentThis, _ctx));
    }
}

void TypeChecking::visit(BoolLiteral* boollit) {
    boollit->setType(_res.Bool());
}

void TypeChecking::visit(IntLiteral* intlit) {
    intlit->setType(_res.Int());
}

void TypeChecking::visit(RealLiteral* reallit) {
    reallit->setType(_res.Real());
}

void TypeChecking::visit(StringLiteral* strlit) {
    strlit->setType(_res.String());
}

TypeChecking::FieldInfo TypeChecking::tryGetFieldInfo(ASTNode* triggerer, ClassDecl* clss, const std::string& id, const type::Environment& env) {
    const auto& it = clss->getScope()->getAllSymbols().equal_range(id);

    if (it.first == it.second) {
        return {nullptr, nullptr};
    }

    auto b = utils::TakeSecondIterator<decltype(it.first)>(it.first);
    auto e = utils::TakeSecondIterator<decltype(it.second)>(it.second);
    const AnySymbolicData data = resolveOverload(triggerer, b, e, env);

    if (data.symbol) {
        type::Type* t = tryGetTypeOfSymbol(data.symbol);
        if (data.env) {
            t = t->substitute(*data.env, _ctx);
        }
        t = t->substitute(env, _ctx);

        return {data.symbol, t};
    } else {
        return {nullptr, nullptr};
    }
}

type::Type* TypeChecking::tryGetTypeOfSymbol(sym::Symbol* sym) {
    if (sym::VariableSymbol* varsym = sym::getIfSymbolOfType<sym::VariableSymbol>(sym)) {
        return varsym->type();
    } else if (sym::DefinitionSymbol* defsym = sym::getIfSymbolOfType<sym::DefinitionSymbol>(sym)) {
        defsym->getDef()->onVisit(this);

        if (defsym->type() == type::Type::NotYetDefined()) {
            _rep.error(*defsym, "Type of " + defsym->getName() + " cannot be inferred because of a cyclic dependency");
        }

        return defsym->type();
    }
    return nullptr;
}

bool TypeChecking::transformIntoCallToMember(FunctionCall* call, Expression* newCallee, type::ProperType* pt, const std::string& member,
                                             const std::vector<TypeExpression*>& typeArgs, ArgTypeEvaluator* callArgTypes,
                                             const std::vector<type::Type*>*& expectedArgTypes, type::Type*& retType) {
    ClassDecl* clss = pt->getClass();
    const type::Environment& env = pt->getEnvironment();

    FieldInfo field = tryGetFieldInfo(newCallee, clss, member, env);

    if (field.isValid()) {
        type::Type* calleeT = ASTTypeCreator::evalFunctionConstructor(field.t->applyTCCallsOnly(_ctx), typeArgs, *call, _ctx, callArgTypes);

        if (type::FunctionType* ft = type::getIf<type::FunctionType>(calleeT)) {
            expectedArgTypes = &ft->getArgTypes();
            retType = ft->getRetType();
        } else if (type::MethodType* mt = type::getIf<type::MethodType>(calleeT)) {
            expectedArgTypes = &mt->getArgTypes();
            retType = mt->getRetType();

            MemberAccess* dot = _mngr.New<MemberAccess>(newCallee, _mngr.New<Identifier>(member));
            dot->setSymbol(field.s);
            dot->setType(field.t);

            common::Positionnable callPos(*call);
            *call = FunctionCall(dot, call->getTypeArgsTuple(), call->getArgsTuple());
            call->setPos(callPos);

            return true;
        } else {
            _rep.error(*newCallee, "Member `" + member + "` of class " + clss->getName() + " is not callable");
        }
    } else {
        _rep.error(*newCallee, "No member named `" + member + "` in class " + clss->getName());
    }

    return false;
}

template<typename T>
void TypeChecking::tryAssigningTypeToSymbolic(T* symbolic) {
    const AnySymbolicData data = resolveOverload(symbolic, symbolic->getSymbolDatas().cbegin(), symbolic->getSymbolDatas().cend(), {});
    if (sym::Symbol* sym = data.symbol) {
        symbolic->setSymbol(sym);
        if (type::Type* t = tryGetTypeOfSymbol(sym)) {
            symbolic->setType(data.env ? t->substitute(*data.env, _ctx) : t);
        }
    }
}

sym::DefinitionSymbol* TypeChecking::findOverridenSymbol(sym::DefinitionSymbol* def) {
    sym::Scoped* scoped;

    if (ClassDecl* clss = getIfNodeOfType<ClassDecl>(def->getOwner(), _ctx)) {
        scoped = clss;
    } else {
        _rep.fatal(*def, "Owner of " + def->getName() + " is unexpected");
        return nullptr;
    }

    const auto& itPair = scoped->getScope()->getAllSymbols().equal_range(def->getName());
    type::Type* defType = def->type()->apply(_ctx);

    for (auto it = itPair.first; it != itPair.second; ++it) {
        const sym::SymbolData& data = it->second;

        if (data.symbol != def) {
            if (sym::DefinitionSymbol* potentialDef = sym::getIfSymbolOfType<sym::DefinitionSymbol>(data.symbol)) {
                if (defType->isSubTypeOf(potentialDef->type()->substitute(data.env, _ctx)->apply(_ctx), _ctx)) {
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
    }

    _rep.error(*def, "Could not find the definition overriden by " +
               def->getName() + " (which has type " + def->type()->apply(_ctx)->toString() + ")");

    return nullptr;
}

void TypeChecking::assignFunctionType(FunctionCreation* func,
                                      const std::vector<type::Type*>& argTypes,
                                      type::Type* retType) {

    std::vector<TypeExpression*> typeArgs;
    ClassDecl* funcClass;
    type::Type* funcType;

    FunctionCreation* meth = _mngr.New<FunctionCreation>("()", func->getTypeArgs(), func->getArgs(), func->getBody(), func->getReturnType());
    DefineDecl* funcDecl;
    sym::Scope* funcClassScope = _mngr.New<sym::Scope>(func->getScope()->getParent(), true);

    type::Environment env(ASTTypeCreator::buildEnvironmentFromTypeParametrizable(func));

    if (func->getTypeArgs()) {
        typeArgs = func->getTypeArgs()->getExpressions();
        funcDecl   = _mngr.New<DefineDecl>(_mngr.New<Identifier>("()"), nullptr, meth, false, false, false);
        funcClass   = _mngr.New<ClassDecl>(func->getName(), nullptr, std::vector<TypeDecl*>(),
                                                          std::vector<TypeSpecifier*>(), std::vector<DefineDecl*>{funcDecl}, false);

        funcType = _mngr.New<type::FunctionType>(typeArgs, argTypes, retType, funcClass, env);
    } else {
        std::vector<type::Type*> parentTypeArgs = argTypes;
        parentTypeArgs.push_back(retType);

        std::string parentName = "Func" + utils::T_toString(argTypes.size());
        std::string absoluteParentName = utils::join(_namer.Func(argTypes.size()), ".");
        type::Type* parentType = _mngr.New<type::ConstructorApplyType>(_res.Func(argTypes.size()), parentTypeArgs);
        sym::TypeSymbol* parentSymbol = _mngr.New<sym::TypeSymbol>(parentName, absoluteParentName, nullptr);
        TypeIdentifier* parentExpr = _mngr.New<TypeIdentifier>(parentName);

        parentExpr->setSymbol(parentSymbol);
        parentSymbol->setType(parentType);

        funcDecl   = _mngr.New<DefineDecl>(_mngr.New<Identifier>("()"), nullptr, meth, true, false, false);
        funcClass   = _mngr.New<ClassDecl>(func->getName(), parentExpr, std::vector<TypeDecl*>(),
                                                          std::vector<TypeSpecifier*>(), std::vector<DefineDecl*>{funcDecl}, false);

        funcType = _mngr.New<type::FunctionType>(typeArgs, argTypes, retType, funcClass, env);

        _redefs.push_back(funcDecl);

        if (type::ProperType* parentPT = type::getIf<type::ProperType>(parentType->apply(_ctx))) {
            funcClassScope->copySymbolsFrom(parentPT->getClass()->getScope(), parentPT->getEnvironment(), sym::Scope::ExcludeConstructors);
        } else {
            _rep.fatal(*funcDecl, "Could not create type " + parentName);
        }
    }

    meth->setType(_mngr.New<type::MethodType>(funcClass, typeArgs, argTypes, retType, env));
    meth->setPos(*func);

    sym::DefinitionSymbol* funcSym = _mngr.New<sym::DefinitionSymbol>("()", "", funcDecl, funcClass);

    funcSym->setType(meth->type());
    funcSym->setPos(*func);

    funcDecl->setSymbol(funcSym);
    funcDecl->setPos(*func);

    funcClass->setScope(funcClassScope);
    funcClass->setPos(*func);

    funcClassScope->addSymbol(funcSym);

    func->setType(funcType);

    _visitedDefs.insert(funcDecl);
}

class OverloadedDefSymbolCandidate final {
public:

    OverloadedDefSymbolCandidate() {}

    sym::DefinitionSymbol* symbol() const { return _symbol; }
    const type::Environment* env() const { return _env; }

    bool isRedef() const { return _symbol->getDef()->isRedef(); }

    size_t argCount() const { return _args->size(); }
    type::Type* arg(size_t index) const { return _args->at(index); }

    void incrScore() { ++_score; }
    int32_t score() const { return _score; }

    void invalidate() { _score = -1; }
    bool isValid() const { return _score >= 0; }

    type::Type* appliedType() const { return _appliedType; }

    static void append(std::vector<OverloadedDefSymbolCandidate>& vec, sym::DefinitionSymbol* s, type::Type* t, size_t expectedArgCount,
                       const type::Environment& env, const type::Environment* dataEnv, CompCtx_Ptr& ctx)
    {
        if (type::FunctionType* ft = type::getIf<type::FunctionType>(t)) {
            if (ft->getArgTypes().size() == expectedArgCount) {
                vec.push_back(OverloadedDefSymbolCandidate(s, dataEnv, applyEnvsHelper(ft, env, dataEnv, ctx)));
            }
        } else if (type::MethodType* mt = type::getIf<type::MethodType>(t)) {
            if (mt->getArgTypes().size() == expectedArgCount) {
                vec.push_back(OverloadedDefSymbolCandidate(s, dataEnv, applyEnvsHelper(mt, env, dataEnv, ctx)));
            }
        }
    }

private:

    OverloadedDefSymbolCandidate(sym::DefinitionSymbol* s, const type::Environment* env, type::FunctionType* ft)
        : _symbol(s), _env(env), _args(&ft->getArgTypes()), _ret(ft->getRetType()), _score(0), _appliedType(ft)
    { }

    OverloadedDefSymbolCandidate(sym::DefinitionSymbol* s, const type::Environment* env, type::MethodType* mt)
        : _symbol(s), _env(env), _args(&mt->getArgTypes()), _ret(mt->getRetType()), _score(0), _appliedType(mt)
    { }

    sym::DefinitionSymbol* _symbol;
    const type::Environment* _env;

    const std::vector<type::Type*>* _args;
    type::Type* _ret;
    int32_t _score;

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
        const type::Environment& env)
{
    size_t dist = std::distance(begin, end);
    if (dist == 1) {
        const auto& val = *begin;
        return AnySymbolicData(val.symbol, val.env);
    } else if (dist == 2) {

        // These happen often when calling a free function, because the abstract `()`
        // operator is overriden by the concrete implementation, and therefore
        // we need to resolve the `()` "overloading" at each function call.

        SymbolIterator beginCopy = begin;

        const auto& val1 = *beginCopy;
        const auto& val2 = *(++beginCopy);

        if (sym::DefinitionSymbol* def1 = sym::getIfSymbolOfType<sym::DefinitionSymbol>(val1.symbol)) {
            if (sym::DefinitionSymbol* def2 = sym::getIfSymbolOfType<sym::DefinitionSymbol>(val2.symbol)) {
                if      (!def1->getDef()->isRedef() && def2->getDef()->isRedef()) { return {val2.symbol, val2.env}; }
                else if (!def2->getDef()->isRedef() && def1->getDef()->isRedef()) { return {val1.symbol, val1.env}; }
            }
        }
    } else if (_expectedInfo.node != triggerer) {
        _rep.error(*triggerer, "Not enough information are provided to determine the right symbol");
        return {nullptr, nullptr};
    }

    // Register potential candidates

    std::vector<OverloadedDefSymbolCandidate> candidates;
    size_t expectedArgCount = _expectedInfo.args->size();

    for (SymbolIterator it = begin; it != end; ++it) {
        const auto& val = *it;
        const AnySymbolicData data(val.symbol, val.env);
        if (sym::DefinitionSymbol* defsymbol = sym::getIfSymbolOfType<sym::DefinitionSymbol>(data.symbol)) {
            type::Type* deftype = ASTTypeCreator::evalFunctionConstructor(defsymbol->type(), *_expectedInfo.typeArgs, *triggerer, _ctx, _expectedInfo.args, false);
            OverloadedDefSymbolCandidate::append(candidates, defsymbol, deftype, expectedArgCount, env, data.env, _ctx);
        }
    }

    size_t candidateCount = candidates.size();

    // Some shortcuts

    if (candidateCount == 1) {
        // This case allows to return early if the overloading was done
        // only on the number of parameters of the function

        return {candidates[0].symbol(), candidates[0].env()};
    }

    // Normal path: Compute the score of each candidate
    // and fill the table of redefinition

    bool candidateRedefs[candidateCount][candidateCount] = {};
    for (size_t i = 0; i < candidateCount; ++i) {
        // At first, set redefs to override everything
        bool iRedefsj = candidates[i].isRedef();
        for (size_t j = 0; j < candidateCount; ++j) {
            candidateRedefs[i][j] = iRedefsj;
        }
    }

    for (size_t a = 0; a < expectedArgCount; ++a) {
        for (size_t i = 0; i < candidateCount; ++i) {
            type::Type* iType = candidates[i].arg(a);

            for (size_t j = i + 1; j < candidateCount; ++j) {
                type::Type* jType = candidates[j].arg(a);

                if (iType->isSubTypeOf(jType, _ctx)) { candidates[i].incrScore(); }
                else { candidateRedefs[j][i] = false; }

                if (jType->isSubTypeOf(iType, _ctx)) { candidates[j].incrScore(); }
                else { candidateRedefs[i][j] = false; }
            }
        }
    }

#ifdef DEBUG_FUNCTION_OVERLOADING
    debugDumpCandidateScores(candidates, _ctx);
#endif

    // Only keep the best redefs

    for (size_t i = 0; i < candidateCount; ++i) {
        for (size_t j = i + 1; j < candidateCount; ++j) {
            if (candidateRedefs[i][j]) {
                candidates[j].invalidate();
            } else if (candidateRedefs[j][i]) {
                candidates[i].invalidate();
            }
        }
    }

    for (auto candidate = candidates.begin(); candidate != candidates.end();) {
        if (!candidate->isValid()) {
            candidate = candidates.erase(candidate);
        } else {
            ++candidate;
        }
    }

    // Small shortcut

    if (candidates.size() == 1) {
        return {candidates[0].symbol(), candidates[0].env()};
    }

    // Discard candidates that are not compatible with the provided argument types

    for (size_t a = 0; a < expectedArgCount; ++a) {
        for (auto candidate = candidates.begin(); candidate != candidates.end();) {
            if (_expectedInfo.args->at(a)->apply(_ctx)->isSubTypeOf(candidate->arg(a), _ctx)) {
                ++candidate;
            } else {
                candidate = candidates.erase(candidate);
            }
        }

        // If at any point there is only one candidate left, return it
        if (candidates.size() == 1) {
            return {candidates[0].symbol(), candidates[0].env()};
        }
    }

    std::vector<OverloadedDefSymbolCandidate*> theChosenOnes;

    // If there are still several candidate, find those which share the best score
    if (candidates.size() > 0) {
        int32_t maxScore = candidates[0].score();

        for (OverloadedDefSymbolCandidate& candidate : candidates) {
            if (candidate.score() == maxScore) {
                theChosenOnes.push_back(&candidate);
            } else if (candidate.score() > maxScore) {
                theChosenOnes.resize(1);
                theChosenOnes[0] = &candidate;
                maxScore = candidate.score();
            }
        }
    }

    OverloadedDefSymbolCandidate* chosen = theChosenOnes.empty() ? nullptr : theChosenOnes.front();

    switch (theChosenOnes.size()) {
    case 0:
        _rep.error(*triggerer, "No viable candidate found among " + utils::T_toString(candidateCount) + " overloads");
        return {nullptr, nullptr};

    default:
        // If there are more than one candidate sharing the best score, then it is an ambiguous access

        _rep.error(*triggerer, "Ambiguous symbol access. Multiple candidates match the requirements:");
        for (OverloadedDefSymbolCandidate* candidate : theChosenOnes) {
            _rep.info(*candidate->symbol(), "Is a viable candidate (has type " + candidate->appliedType()->toString() + ")");
        }

    case 1:
        return {chosen->symbol(), chosen->env()};
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
