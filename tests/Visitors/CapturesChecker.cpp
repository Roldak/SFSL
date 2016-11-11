//
//  CapturesChecker.cpp
//  SFSL
//
//  Created by Romain Beguet on 11.11.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#include "CapturesChecker.h"
#include "Compiler/Backend/AST2BAST/PreTransform.h"

namespace sfsl {

namespace test {

CapturesChecker::CapturesChecker(CompCtx_Ptr& ctx) : ASTImplicitVisitor(ctx) {

}

CapturesChecker::~CapturesChecker() {

}

void CapturesChecker::visit(ast::ClassDecl* clss) {
    visitClosure(*clss, clss, clss);

    ASTImplicitVisitor::visit(clss);
}

void CapturesChecker::visit(ast::FunctionCreation* func) {
    if (type::ProperType* pt = type::getIf<type::ProperType>(func->type())) {
        visitClosure(*func, func, pt->getClass());
    }

    ASTImplicitVisitor::visit(func);
}

void CapturesChecker::visitClosure(const common::Positionnable& pos, ast::Annotable* annotable,
                                   common::HasManageableUserdata* closureData) {

    if (ast::ClassPatch* patch = closureData->getUserdata<ast::ClassPatch>()) {
        for (const ast::Change& c : patch->getChanges()) {
            bool ok = false;

            annotable->matchAnnotation<std::string, bool>("captures", [&](std::string name, bool mut) {
                if (c.newField->getValue() == name) {
                    ok = true;
                }
            });

            if (!ok) {
                _ctx->reporter().error(pos, c.newField->getValue() + " is captured but not checked");
            }
        }
    }
}

}

}
