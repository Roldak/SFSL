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

namespace sfsl {

namespace ast {

    /**
     * @brief
     */
    class TypeAssignation : public ASTVisitor {
    public:

        TypeAssignation(std::shared_ptr<common::CompilationContext>& ctx);

        virtual void visit(ASTNode*);

        virtual void visit(ModuleDecl* mod);
        virtual void visit(ClassDecl* clss);
        virtual void visit(DefineDecl* decl);

        virtual void visit(Block* block);
        virtual void visit(FunctionCreation* func);

        virtual void visit(TypeSpecifier *tps);

        virtual void visit(IntLitteral* intlit);
        virtual void visit(RealLitteral* reallit);

    private:

        sym::Scope* _curScope;

    };

    /**
     * @brief
     */
    class TypeCheck : public ASTVisitor {
    public:

        TypeCheck(std::shared_ptr<common::CompilationContext>& ctx);

        virtual void visit(Statement* stmt);

        virtual void visit(MemberAccess* mac);

    private:

    };
}

}

#endif
