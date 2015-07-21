//
//  CodeGenerator.h
//  SFSL
//
//  Created by Romain Beguet on 21.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__CodeGenerator__
#define __SFSL__CodeGenerator__

#include <iostream>
#include <set>
#include "../AST/Visitors/ASTVisitor.h"
#include "../AST/Symbols/SymbolResolver.h"

namespace sfsl {

namespace ast {

    /**
     * @brief
     */
    class CodeGenerator : public ASTVisitor {
    public:

        CodeGenerator(CompCtx_Ptr& ctx);
        virtual ~CodeGenerator();

        virtual void visit(ASTNode*) override;

        virtual void visit(TypeDecl* tdecl) override;
        virtual void visit(DefineDecl* decl) override;

        virtual void visit(ExpressionStatement* exp) override;

        virtual void visit(BinaryExpression* bin) override;
        virtual void visit(AssignmentExpression* aex) override;
        virtual void visit(TypeSpecifier* tps) override;
        virtual void visit(Block* block) override;
        virtual void visit(IfExpression* ifexpr) override;
        virtual void visit(MemberAccess* dot) override;
        virtual void visit(Tuple* tuple) override;
        virtual void visit(FunctionCreation* func) override;
        virtual void visit(FunctionCall* call) override;
        virtual void visit(Identifier* ident) override;
        virtual void visit(IntLitteral* intlit) override;
        virtual void visit(RealLitteral* reallit) override;

    private:
    };
}

}

#endif
