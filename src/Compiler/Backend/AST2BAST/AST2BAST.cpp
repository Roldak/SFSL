//
//  AST2BAST.cpp
//  SFSL
//
//  Created by Romain Beguet on on 28.03.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#include "AST2BAST.h"
#include "../../Frontend/AST/Visitors/ASTSymbolExtractor.h"
#include "../../Frontend/AST/Visitors/ASTTypeIdentifier.h"

namespace sfsl {

namespace bast {

AST2BAST::AST2BAST(CompCtx_Ptr& ctx) : ASTExplicitVisitor(ctx), _freshId(0), _rep(ctx->reporter()), _created(nullptr) {

}

AST2BAST::~AST2BAST() {

}

void AST2BAST::visit(ast::ASTNode*) {

}

void AST2BAST::visit(ast::Program* prog) {
    for (ast::ModuleDecl* module : prog->getModules()) {
        module->onVisit(this);
    }
    make<Program>(_defs);
}

void AST2BAST::visit(ast::ModuleDecl* module) {
    for (ast::ModuleDecl* mod : module->getSubModules()) {
        mod->onVisit(this);
    }

    for (ast::TypeDecl* type : module->getTypes()) {
        type->onVisit(this);
    }

    for (ast::DefineDecl* decl : module->getDeclarations()) {
        decl->onVisit(this);
    }
}

void AST2BAST::visit(ast::TypeDecl* tdecl) {
    _defs.push_back(make<GlobalDef>(tdecl->getSymbol()->getAbsoluteName(), transform(tdecl->getExpression())));
}

void AST2BAST::visit(ast::ClassDecl* clss) {
    ast::ClassUserData* classUD = clss->getUserdata<ast::ClassUserData>();

    if (!alreadyTransformed(classUD)) {
        if (clss->getParent()) {
            clss->getParent()->onVisit(this);
        }

        for (ast::TypeDecl* tdecl: clss->getTypeDecls()) {
            tdecl->onVisit(this);
        }

        for (ast::DefineDecl* decl : clss->getDefs()) {
            decl->onVisit(this);
        }

        std::vector<DefIdentifier*> methods;

        for (sym::DefinitionSymbol* def : classUD->getDefs()) {
            methods.push_back(make<DefIdentifier>(def->getUserdata<ast::DefUserData>()->getDefId()));
        }

        _defs.push_back(make<ClassDef>(classUD->getDefId(), classUD->getAttrCount(), methods));
    }

    make<DefIdentifier>(classUD->getDefId());
}

void AST2BAST::visit(ast::DefineDecl* decl) {
    if (decl->isExtern() || decl->isAbstract()) {
        return;
    }

    _defs.push_back(make<GlobalDef>(decl->getSymbol()->getAbsoluteName(), transform(decl->getValue())));
}

void AST2BAST::visit(ast::ProperTypeKindSpecifier* ptks) {
    makeBad();
}

void AST2BAST::visit(ast::TypeConstructorKindSpecifier* tcks) {
    makeBad();
}

void AST2BAST::visit(ast::FunctionTypeDecl* ftdecl) {

}

void AST2BAST::visit(ast::TypeMemberAccess* tdot) {
    visitSymbolic(tdot);
}

void AST2BAST::visit(ast::TypeTuple* ttuple) {
    makeBad();
}

void AST2BAST::visit(ast::TypeConstructorCreation* typeconstructor) {
    typeconstructor->getBody()->onVisit(this);
}

void AST2BAST::visit(ast::TypeConstructorCall* tcall) {
    tcall->getCallee()->onVisit(this);
    for (ast::TypeExpression* arg : tcall->getArgs()) {
        arg->onVisit(this);
    }
}

void AST2BAST::visit(ast::TypeIdentifier* tident) {
    visitSymbolic(tident);
}

void AST2BAST::visit(ast::TypeToBeInferred* tbi) {
    makeBad();
}

void AST2BAST::visit(ast::KindSpecifier* ks) {
    makeBad();
}

void AST2BAST::visit(ast::ExpressionStatement* exp) {
    transform(exp->getExpression());
}

void AST2BAST::visit(ast::AssignmentExpression* aex) {

}

void AST2BAST::visit(ast::TypeSpecifier* tps) {
    makeUnit();
}

void AST2BAST::visit(ast::Block* block) {
    std::vector<Expression*> bexprs;
    for (ast::Expression* expr : block->getStatements()) {
        bexprs.push_back(transform(expr));
    }
    make<Block>(bexprs);
}

void AST2BAST::visit(ast::IfExpression* ifexpr) {
    Expression* cond = transform(ifexpr->getCondition());
    Expression* then = transform(ifexpr->getThen());
    Expression* els = ifexpr->getElse()
            ? transform(ifexpr->getElse())
            : make<Block>(std::vector<Expression*>());

    make<IfExpression>(cond, then, els);
}

void AST2BAST::visit(ast::MemberAccess* dot) {
    if (dot->getSymbol()->getSymbolType() == sym::SYM_VAR) {
        size_t fieldLoc = getVarLoc(static_cast<sym::VariableSymbol*>(dot->getSymbol()));
        make<FieldAccess>(transform(dot->getAccessed()), fieldLoc);
    } else {
        visitSymbolic(dot);
    }
}

void AST2BAST::visit(ast::Tuple* tuple) {
    makeUnit();
}

void AST2BAST::visit(ast::FunctionCreation* func) {
    ast::FuncUserData* fUD = func->getUserdata<ast::FuncUserData>();

    if (type::ProperType* pt = type::getIf<type::ProperType>(func->type())) {
        transform(pt->getClass());
        // TODO: Call the constructor of the function's class
        make<Instantiation>(make<DefIdentifier>(getDefId(pt->getClass())));
    } else {
        _defs.push_back(make<MethodDef>(fUD->getDefId(), fUD->getVarCount(), transform(func->getBody())));
        make<DefIdentifier>(fUD->getDefId());
    }
}

void AST2BAST::visit(ast::FunctionCall* call) {
    if (call->getCallee()->type()->getTypeKind() == type::TYPE_METHOD) {
        sym::DefinitionSymbol* def = static_cast<sym::DefinitionSymbol*>(ast::ASTSymbolExtractor::extractSymbol(call->getCallee(), _ctx));
        size_t virtualLoc = def->getUserdata<ast::VirtualDefUserData>()->getVirtualLocation();

        Expression* callee;
        std::vector<Expression*> args;

        if (ast::isNodeOfType<ast::MemberAccess>(call->getCallee(), _ctx)) {
            ast::MemberAccess* dot = static_cast<ast::MemberAccess*>(call->getCallee());
            callee = transform(dot->getAccessed());
        } else if (ast::isNodeOfType<ast::Identifier>(call->getCallee(), _ctx)) { // implicit this
            callee = make<VarIdentifier>(0);
        }

        for (ast::Expression* expr : call->getArgs()) {
            args.push_back(transform(expr));
        }

        make<MethodCall>(callee, virtualLoc, args);
    }
}

void AST2BAST::visit(ast::Instantiation* inst) {
    std::string name = freshName(freshName("inst"));
    _defs.push_back(make<GlobalDef>(name, transform(inst->getInstantiatedExpression())));
    make<Instantiation>(make<DefIdentifier>(name));
}

void AST2BAST::visit(ast::Identifier* ident) {
    visitSymbolic(ident);
}

void AST2BAST::visit(ast::This*) {
    make<VarIdentifier>(0);
}

void AST2BAST::visit(ast::BoolLitteral* boollit) {
    make<BoolLitteral>(boollit->getValue());
}

void AST2BAST::visit(ast::IntLitteral* intlit) {
    make<IntLitteral>(intlit->getValue());
}

void AST2BAST::visit(ast::RealLitteral* reallit) {
    make<RealLitteral>(reallit->getValue());
}

void AST2BAST::visit(ast::StringLitteral* strlit) {
    make<StringLitteral>(strlit->getValue());
}

Program* AST2BAST::transform(ast::Program* node) {
    node->onVisit(this);
    return static_cast<Program*>(_created);
}

template<typename T>
void AST2BAST::visitSymbolic(T* symbolic) {
    if (symbolic->getSymbolCount() != 1) {
        _rep.error(*symbolic, "Identifier refers to several symbols");
        makeBad();
    } else {
        switch (symbolic->getSymbol()->getSymbolType()) {
        case sym::SYM_VAR: {
            sym::VariableSymbol* var = static_cast<sym::VariableSymbol*>(symbolic->getSymbol());
            if (isVariableAttribute(var)) {
                make<FieldAccess>(make<VarIdentifier>(0), getVarLoc(var));
            } else {
                make<VarIdentifier>(getVarLoc(var));
            }
            break;
        }

        case sym::SYM_DEF:
        case sym::SYM_TPE: {
            make<DefIdentifier>(symbolic->getSymbol()->getAbsoluteName());
            break;
        }

        default:
            break;
        }
    }
}

const std::string& AST2BAST::getDefId(ast::ClassDecl* clss) {
    return clss->getUserdata<ast::ClassUserData>()->getDefId();
}

const std::string& AST2BAST::getDefId(sym::DefinitionSymbol* def) {
    return def->getUserdata<ast::DefUserData>()->getDefId();
}

size_t AST2BAST::getVarLoc(sym::VariableSymbol* var) {
    return var->getUserdata<ast::VarUserData>()->getVarLoc();
}

bool AST2BAST::isVariableAttribute(sym::VariableSymbol* var) {
    return var->getUserdata<ast::VarUserData>()->isAttribute();
}

Expression* AST2BAST::makeUnit() {
    return make<UnitLitteral>();
}

template<typename T, typename... Args>
T* AST2BAST::make(Args... args) {
    T* toRet = _mngr.New<T>(std::forward<Args>(args)...);
    _created = toRet;
    return toRet;
}

BASTNode* AST2BAST::makeBad() {
    return _created = nullptr;
}

Expression* AST2BAST::transform(ast::Expression* node) {
    node->onVisit(this);
    return static_cast<Expression*>(_created);
}

bool AST2BAST::alreadyTransformed(ast::DefUserData* defUD) {
    for (Definition* def : _defs) {
        if (def->getName() == defUD->getDefId()) {
            return true;
        }
    }
    return false;
}

std::string AST2BAST::freshName(const std::string& prefix) {
    return prefix + "$$" + std::to_string(_freshId++);
}

}

}
