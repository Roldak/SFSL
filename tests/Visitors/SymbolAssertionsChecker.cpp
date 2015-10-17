//
//  SymbolAssertionsChecker.cpp
//  SFSL
//
//  Created by Romain Beguet on 02.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "SymbolAssertionsChecker.h"
#include "Compiler/Frontend/AST/Visitors/ASTTypeIdentifier.h"
#include "Compiler/Frontend/AST/Visitors/ASTSymbolExtractor.h"

namespace sfsl {

namespace test {

SymbolAssertionsChecker::SymbolAssertionsChecker(CompCtx_Ptr& ctx) : ASTImplicitVisitor(ctx) {

}

SymbolAssertionsChecker::~SymbolAssertionsChecker() {

}

void SymbolAssertionsChecker::visit(ast::ModuleDecl* module) {
    tryAddTestSymbol(module->getSymbol());
    ASTImplicitVisitor::visit(module);
}

void SymbolAssertionsChecker::visit(ast::TypeDecl* tdecl) {
    tryAddTestSymbol(tdecl->getSymbol());
    ASTImplicitVisitor::visit(tdecl);
}

void SymbolAssertionsChecker::visit(ast::DefineDecl* decl) {
    tryAddTestSymbol(decl->getSymbol());
    ASTImplicitVisitor::visit(decl);
}

void SymbolAssertionsChecker::visit(ast::KindSpecifier* ks) {
    tryAddTestSymbol(ks->getSpecified()->getSymbol());
    ASTImplicitVisitor::visit(ks);
}

void SymbolAssertionsChecker::visit(ast::FunctionCall* call) {
    if (ast::isNodeOfType<ast::Identifier>(call->getCallee(), _ctx)) {
        ast::Identifier* id = static_cast<ast::Identifier*>(call->getCallee());
        if (id->getValue() == ASSERT_SAME_SYM) {
            const std::vector<ast::Expression*>& args(call->getArgs());
            if (args.size() != 2) {
                _ctx->reporter().fatal(*call, "Expected 2 arguments, got " + utils::T_toString(args.size()));
                return;
            }

            if (ast::isNodeOfType<ast::StringLitteral>(args[0], _ctx)) {
                const std::string& symName = static_cast<ast::StringLitteral*>(args[0])->getValue();
                if (sym::Symbol* s = ast::ASTSymbolExtractor::extractSymbol(args[1], _ctx)) {
                    auto it = _symbols.find(symName);
                    if (it == _symbols.end()) {
                        _ctx->reporter().error(*(args[0]), "No symbol named " + symName + " were registed for testing");
                    } else if (it->second != s) {
                        _ctx->reporter().error(*call, "The two symbols do not match. Expected " +
                                               it->second->getName() + ", found " + s->getName());
                    }
                } else {
                    _ctx->reporter().error(*(args[1]), "No symbol was assigned");
                }

            } else {
                _ctx->reporter().fatal(*(args[0]), "Expected string litteral");
            }
            return;
        }
    }
    ASTImplicitVisitor::visit(call);
}

void SymbolAssertionsChecker::visit(ast::TypeSpecifier* tps) {
    tryAddTestSymbol(tps->getSpecified()->getSymbol());
    ASTImplicitVisitor::visit(tps);
}

sym::Symbol*& SymbolAssertionsChecker::findSymbolLocation(const std::string& name, size_t index) {
    if (sym::Symbol*& loc = _symbols[name + "_" + utils::T_toString(index)]) {
        return findSymbolLocation(name, index + 1);
    } else {
        return loc;
    }
}

void SymbolAssertionsChecker::tryAddTestSymbol(sym::Symbol* s) {
    if (s->getName().substr(0, 5) == "test_") {
        findSymbolLocation(s->getName(), 0) = s;
    }
}

}

}
