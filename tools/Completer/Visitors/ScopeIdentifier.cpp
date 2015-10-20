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

ScopeIdentifer::ScopeIdentifer(CompCtx_Ptr& ctx, AbstractScopeReporter* rep)
    : ASTImplicitVisitor(ctx), _rep(rep), _curId(0) {

}

ScopeIdentifer::~ScopeIdentifer() {

}

void ScopeIdentifer::visit(ast::Program* prog) {
    saveAndReport(prog);

    ASTImplicitVisitor::visit(prog);
}

void ScopeIdentifer::visit(ast::ModuleDecl* module) {
    saveAndReport(module);

    ASTImplicitVisitor::visit(module);

    restore();
}

void ScopeIdentifer::visit(ast::TypeDecl* tdecl) {
    saveAndReport(tdecl);

    ASTImplicitVisitor::visit(tdecl);

    restore();
}

void ScopeIdentifer::visit(ast::ClassDecl* clss) {
    saveAndReport(clss);

    ASTImplicitVisitor::visit(clss);

    restore();
}

void ScopeIdentifer::visit(ast::DefineDecl* decl) {
    saveAndReport(decl);

    ASTImplicitVisitor::visit(decl);

    restore();
}

void ScopeIdentifer::visit(ast::TypeConstructorCreation* tc) {
    saveAndReport(tc);

    ASTImplicitVisitor::visit(tc);

    restore();
}

void ScopeIdentifer::visit(ast::Block* block) {
    saveAndReport(block);

    ASTImplicitVisitor::visit(block);

    restore();
}

void ScopeIdentifer::visit(ast::FunctionCreation* func) {
    saveAndReport(func);

    ASTImplicitVisitor::visit(func);

    restore();
}

void ScopeIdentifer::report(common::Positionnable* pos) {
    _rep->reportScope(pos, _curScopePath);
}

void ScopeIdentifer::saveAndReport(common::Positionnable* pos) {
    save();
    report(pos);
}

void ScopeIdentifer::save() {
    _curScopePath.push_back(_curId);
    _curId = 0;
}

void ScopeIdentifer::restore() {
    _curId = _curScopePath.back() + 1;
    _curScopePath.pop_back();
}

}

}
