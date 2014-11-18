//
//  ASTVisitorTemplate.h
//  SFSL
//
//  Created by Romain Beguet on 17.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ASTVisitorTemplate__
#define __SFSL__ASTVisitorTemplate__

#include <iostream>
#include "ASTVisitor.h"

namespace sfsl {

namespace ast {

/**
 * @brief Provides a basic template for future ASTVisitors implementations
 */
class ASTVisitorTemplate : public ASTVisitor {
public:

    ASTVisitorTemplate(std::shared_ptr<common::CompilationContext>& ctx);

    virtual void visit(Program* prog);

    virtual void visit(ModuleDecl* module);
    virtual void visit(DefineDecl* decl);

    virtual void visit(BinaryExpression* exp);
    virtual void visit(FunctionCall* call);
    virtual void visit(Identifier* ident);
    virtual void visit(IntLitteral* intlit);
    virtual void visit(RealLitteral* reallit);
};

}

}

#endif
