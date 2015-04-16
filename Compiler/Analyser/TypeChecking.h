//
//  TypeChecking.h
//  SFSL
//
//  Created by Romain Beguet on 30.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__TypeChecking__
#define __SFSL__TypeChecking__

#include <iostream>
#include "../AST/Visitors/ASTVisitor.h"
#include "../AST/Symbols/SymbolResolver.h"

namespace sfsl {

namespace ast {

    /**
     * @brief
     */
    class TypeCheking : public ASTVisitor {
    public:

        TypeCheking(CompCtx_Ptr& ctx, const sym::SymbolResolver& res);

        virtual void visit(ASTNode*);

        virtual void visit(ModuleDecl* mod);
        virtual void visit(ClassDecl* clss);
        virtual void visit(DefineDecl* decl);

        virtual void visit(ExpressionStatement* exp);

        virtual void visit(BinaryExpression* bin);
        virtual void visit(TypeSpecifier* tps);
        virtual void visit(Block* block);
        virtual void visit(IfExpression* ifexpr);
        virtual void visit(MemberAccess* dot);
        virtual void visit(Tuple* tuple);
        virtual void visit(FunctionCreation* func);
        virtual void visit(FunctionCall* call);
        virtual void visit(Identifier* ident);
        virtual void visit(IntLitteral* intlit);
        virtual void visit(RealLitteral* reallit);

    private:

        sym::Scope* _curScope;
        const sym::SymbolResolver& _res;
        common::AbstractReporter& _rep;
    };
}

}

#endif
