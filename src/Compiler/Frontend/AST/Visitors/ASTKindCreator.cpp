//
//  ASTKindCreator.cpp
//  SFSL
//
//  Created by Romain Beguet on 14.05.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "ASTKindCreator.h"
#include "../../Symbols/Scope.h"
#include "ASTTypeCreator.h"

namespace sfsl {

namespace ast {

// AST KIND CREATOR

ASTKindCreator::ASTKindCreator(CompCtx_Ptr& ctx)
    : ASTImplicitVisitor(ctx), _created(nullptr) {

}

ASTKindCreator::~ASTKindCreator() {

}

void ASTKindCreator::visit(ASTNode* node) {
    // do not throw an exception
}

type::Type* ASTKindCreator::computeBoundType(TypeExpression* b) {
    if (b) {
        if (type::Type* tp = ASTTypeCreator::createType(b, _ctx)) {
            if (tp->applyTCCallsOnly(_ctx)->getTypeKind() == type::TYPE_PROPER) {
                return tp;
            } else {
                _ctx->reporter().error(*b, "Proper kind can only have bounds of a proper type");
            }
        }
    }
    return nullptr;
}

void ASTKindCreator::visit(ProperTypeKindSpecifier* ptks) {
    type::Type* lbType = computeBoundType(ptks->getLowerBoundExpr());
    type::Type* ubType = computeBoundType(ptks->getUpperBoundExpr());

    _created = (!lbType && !ubType) ? kind::ProperKind::create()
                                    : _mngr.New<kind::ProperKind>(lbType, ubType);
}

void ASTKindCreator::visit(TypeConstructorKindSpecifier* tcks) {
    std::vector<kind::TypeConstructorKind::Parameter> params(tcks->getArgs().size());
    kind::Kind* ret;

    for (size_t i = 0; i < params.size(); ++i) {
        const TypeConstructorKindSpecifier::Parameter& tcParam(tcks->getArgs()[i]);
        tcParam.kindExpr->onVisit(this);

        if (!_created) {
            return;
        }

        params[i].varianceType = tcParam.varianceType;
        params[i].kind = _created;
    }

    tcks->getRet()->onVisit(this);

    if (!(ret = _created)) {
        return;
    }

    _created = _mngr.New<kind::TypeConstructorKind>(params, ret);
}

kind::Kind* ASTKindCreator::getCreatedKind() const {
    return _created;
}

kind::Kind* ASTKindCreator::createKind(ASTNode* node, CompCtx_Ptr& ctx) {
    ASTKindCreator creator(ctx);
    node->onVisit(&creator);
    return creator.getCreatedKind();
}

// AST DEFAULT TYPE FROM KIND CREATOR

ASTDefaultTypeFromKindCreator::ASTDefaultTypeFromKindCreator(CompCtx_Ptr& ctx, const std::string& name)
    : ASTImplicitVisitor(ctx), _name(name) {

}

ASTDefaultTypeFromKindCreator::ASTDefaultTypeFromKindCreator(CompCtx_Ptr& ctx, const std::string& name,
                                                             const std::vector<Parameter>& dependencies)
    : ASTImplicitVisitor(ctx), _name(name), _parameters(dependencies) {

}

ASTDefaultTypeFromKindCreator::~ASTDefaultTypeFromKindCreator() {

}

void ASTDefaultTypeFromKindCreator::visit(ASTNode*) {
    // do not throw an exception
}

void ASTDefaultTypeFromKindCreator::visit(ProperTypeKindSpecifier*) {
    ClassDecl* clss = _mngr.New<ClassDecl>(_name, nullptr,
                                           std::vector<TypeDecl*>(),
                                           std::vector<TypeSpecifier*>(),
                                           std::vector<DefineDecl*>(), false);

    clss->setScope(_mngr.New<sym::Scope>(nullptr));
    clss->setParameters(_parameters);

    clss->CanSubtypeClasses::insertParent(clss);

    _created = clss;
}

void ASTDefaultTypeFromKindCreator::visit(TypeConstructorKindSpecifier* tcks) {
    std::vector<TypeExpression*> args(tcks->getArgs().size());
    std::vector<Parameter> dependencies(tcks->getArgs().size());
    TypeExpression* ret;

    for (size_t i = 0; i < args.size(); ++i) {
        const TypeConstructorKindSpecifier::Parameter& tcksParam(tcks->getArgs()[i]);
        TypeDecl* tdecl = createDefaultTypeFromKind(tcksParam.kindExpr, _name + "Arg" + utils::T_toString(i), _parameters, _ctx);

        args[i] = _mngr.New<TypeParameter>(tcksParam.varianceType, tdecl->getName(), tcksParam.kindExpr);
        dependencies[i].varianceType = tcksParam.varianceType;
        dependencies[i].symbol = tdecl->getSymbol();
    }

    pushTypeParameters(dependencies);

    ret = createDefaultTypeFromKind(tcks->getRet(), _name + "Ret", _parameters, _ctx)->getName();

    popTypeParameters(dependencies.size());

    TypeTuple* tt = _mngr.New<TypeTuple>(args);

    TypeConstructorCreation* tcc = _mngr.New<TypeConstructorCreation>(_name, tt, ret);
    tcc->setScope(_mngr.New<sym::Scope>(nullptr));
    tcc->setParameters(_parameters);

    _created = tcc;
}

TypeExpression* ASTDefaultTypeFromKindCreator::getCreatedType() const {
    return _created;
}

void ASTDefaultTypeFromKindCreator::pushTypeParameters(const std::vector<Parameter>& typeParams) {
    for (Parameter typeParam : typeParams) {
        _parameters.push_back(typeParam);
    }
}

void ASTDefaultTypeFromKindCreator::popTypeParameters(size_t pushed) {
    _parameters.resize(_parameters.size() - pushed);
}


TypeDecl* ASTDefaultTypeFromKindCreator::createDefaultTypeFromKind(ASTNode* node, const std::string& name, CompCtx_Ptr& ctx) {
    return createDefaultTypeFromKind(node, name, std::vector<Parameter>(), ctx);
}

TypeDecl* ASTDefaultTypeFromKindCreator::createDefaultTypeFromKind(ASTNode* node, const std::string& name,
                                               const std::vector<Parameter>& dependencies, CompCtx_Ptr& ctx) {
    ASTDefaultTypeFromKindCreator creator(ctx, name, dependencies);
    node->onVisit(&creator);
    TypeExpression* expr = creator.getCreatedType();

    type::Type* t = ASTTypeCreator::createType(expr, ctx);

    TypeDecl* tdecl = ctx->memoryManager().New<TypeDecl>(ctx->memoryManager().New<TypeIdentifier>(name), expr);

    sym::TypeSymbol* ts = ctx->memoryManager().New<sym::TypeSymbol>(name, name, tdecl);

    tdecl->setType(t);
    ts->setType(t);

    tdecl->getName()->setSymbol(ts);
    tdecl->setSymbol(ts);

    return tdecl;
}

type::Type* ASTDefaultTypeFromKindCreator::createDefaultTypeFromKind(ASTNode* node, CompCtx_Ptr& ctx) {
    ASTDefaultTypeFromKindCreator creator(ctx, "tmp");
    node->onVisit(&creator);
    TypeExpression* expr = creator.getCreatedType();

    return ASTTypeCreator::createType(expr, ctx);
}

}

}
