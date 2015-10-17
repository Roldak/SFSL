//
//  SymbolAssertionsChecker.h
//  SFSL
//
//  Created by Romain Beguet on 02.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__SymbolAssertionsChecker__
#define __SFSL__SymbolAssertionsChecker__

#include "Compiler/Frontend/AST/Visitors/ASTImplicitVisitor.h"

#define ASSERT_SAME_SYM "assert_same_sym"

namespace sfsl {

namespace test {

class SymbolAssertionsChecker : public ast::ASTImplicitVisitor {
public:
    SymbolAssertionsChecker(CompCtx_Ptr& ctx);
    virtual ~SymbolAssertionsChecker();

    virtual void visit(ast::ModuleDecl* module) override;
    virtual void visit(ast::TypeDecl* tdecl) override;
    virtual void visit(ast::DefineDecl* decl) override;
    virtual void visit(ast::KindSpecifier* ks) override;
    virtual void visit(ast::FunctionCall* call) override;
    virtual void visit(ast::TypeSpecifier* tps) override;

private:

    sym::Symbol*& findSymbolLocation(const std::string& name, size_t index);
    void tryAddTestSymbol(sym::Symbol* s);

    std::map<std::string, sym::Symbol*> _symbols;
};

}

}

#endif
