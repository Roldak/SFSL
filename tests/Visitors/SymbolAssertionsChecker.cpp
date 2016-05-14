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

void SymbolAssertionsChecker::visit(ast::Program* prog) {
    ASTImplicitVisitor::visit(prog);
    performTests();
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

void SymbolAssertionsChecker::visit(ast::TypeParameter* tparam) {
    tryAddTestSymbol(tparam->getSpecified()->getSymbol());
    ASTImplicitVisitor::visit(tparam);
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
                if (sym::Symbol* s = ast::ASTSymbolExtractor::extractSymbol(args[1], _ctx)) {
                    _tests.push_back(std::make_pair(static_cast<ast::StringLitteral*>(args[0]), s));
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

void SymbolAssertionsChecker::performTests() {
    for (const std::pair<ast::StringLitteral*, sym::Symbol*>& test : _tests) {
        const std::string& str(test.first->getValue());
        auto it = _symbols.find(str);
        if (it == _symbols.end()) {
            _ctx->reporter().error(*(test.first), "No symbol named " + str + " were registed for testing");
        } else if (it->second != test.second) {
            common::Positionnable custom(test.first->getStartPosition(), test.second->getEndPosition(), test.first->getSourceName());
            _ctx->reporter().error(custom, "The two symbols do not match. Expected " +
                                   it->second->getName() + ", found " + test.second->getName());
        }
    }
}

}

}
