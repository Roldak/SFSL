//
//  ScopeIdentifer.cpp
//  SFSL
//
//  Created by Romain Beguet on 20.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "ScopeIdentifier.h"

namespace sfsl {

namespace complete {

// SCOPE WALKER

ScopeWalker::ScopeWalker(CompCtx_Ptr& ctx) : ast::ASTImplicitVisitor(ctx), _curId(0) {

}

ScopeWalker::~ScopeWalker() {

}

void ScopeWalker::visit(ast::ModuleDecl* module) {
    enterScope(module);

    ASTImplicitVisitor::visit(module);

    leaveScope();
}

void ScopeWalker::visit(ast::TypeDecl* tdecl) {
    enterScope(tdecl);

    ASTImplicitVisitor::visit(tdecl);

    leaveScope();
}

void ScopeWalker::visit(ast::ClassDecl* clss) {
    enterScope(clss);

    ASTImplicitVisitor::visit(clss);

    leaveScope();
}

void ScopeWalker::visit(ast::DefineDecl* decl) {
    enterScope(decl);

    ASTImplicitVisitor::visit(decl);

    leaveScope();
}

void ScopeWalker::visit(ast::TypeConstructorCreation* tc) {
    enterScope(tc);

    ASTImplicitVisitor::visit(tc);

    leaveScope();
}

void ScopeWalker::visit(ast::Block* block) {
    enterScope(block);

    ASTImplicitVisitor::visit(block);

    leaveScope();
}

void ScopeWalker::visit(ast::FunctionCreation* func) {
    enterScope(func);

    ASTImplicitVisitor::visit(func);

    leaveScope();
}

void ScopeWalker::enterScope(ast::ASTNode* scope) {
    _curScopePath.push_back(_curId);
    _curId = 0;
}

void ScopeWalker::leaveScope() {
    _curId = _curScopePath.back() + 1;
    _curScopePath.pop_back();
}

// SCOPE IDENTIFIER

ScopeReporter::ScopeReporter(CompCtx_Ptr& ctx, AbstractScopeReporter* rep)
    : ScopeWalker(ctx), _rep(rep) {

}

ScopeReporter::~ScopeReporter() {

}

void ScopeReporter::enterScope(ast::ASTNode* scope) {
    ScopeWalker::enterScope(scope);
    report(scope);
}

void ScopeReporter::leaveScope() {
    ScopeWalker::leaveScope();
}

void ScopeReporter::report(common::Positionnable* pos) {
    _rep->reportScope(pos, _curScopePath);
}

// SCOPE ISOLATOR

ScopeFinder::ScopeFinder(CompCtx_Ptr& ctx, const std::vector<size_t>& scopePath)
    : ScopeWalker(ctx), _scopePath(scopePath), _block(nullptr) {

}

ScopeFinder::~ScopeFinder() {

}

void ScopeFinder::visit(ast::DefineDecl* decl) {
    ScopeWalker::enterScope(decl);

    if (isCurrentScopePrefixOfDest()) {
        if (_curScopePath.size() == _scopePath.size()) {
            _block = _mngr.New<ast::Block>(std::vector<ast::Expression*>{decl->getValue()});
            *decl = ast::DefineDecl(decl->getName(), decl->getTypeSpecifier(), _block, decl->getFlags());
        } else {
            ASTImplicitVisitor::visit(decl);
        }
    }

    ScopeWalker::leaveScope();
}

void ScopeFinder::visit(ast::Block* block) {
    ScopeWalker::enterScope(block);

    if (isCurrentScopePrefixOfDest()) {
        if (_curScopePath.size() == _scopePath.size()) { // path are equals
            _block = block;
        } else {
            ASTImplicitVisitor::visit(block);
        }
    }

    ScopeWalker::leaveScope();
}

void ScopeFinder::visit(ast::FunctionCreation* func) {
    ScopeWalker::enterScope(func);

    if (isCurrentScopePrefixOfDest()) {
        ASTImplicitVisitor::visit(func);
    }

    ScopeWalker::leaveScope();
}

ast::Block* ScopeFinder::getBlock() {
    return _block;
}

bool ScopeFinder::isCurrentScopePrefixOfDest() {
    if (_curScopePath.size() > _scopePath.size()) {
        return false;
    } else {
        for (size_t i = 0; i < _curScopePath.size(); ++i) {
            if (_curScopePath[i] != _scopePath[i]) {
                return false;
            }
        }
        return true;
    }
}

}

}
