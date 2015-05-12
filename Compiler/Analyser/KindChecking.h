//
//  KindChecking.h
//  SFSL
//
//  Created by Romain Beguet on 12.05.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__KindChecking__
#define __SFSL__KindChecking__

#include <iostream>
#include <set>
#include "../AST/Visitors/ASTVisitor.h"
#include "../AST/Symbols/SymbolResolver.h"

namespace sfsl {

namespace ast {

    /**
     * @brief
     */
    class KindChecking : public ASTVisitor {
    public:

        KindChecking(CompCtx_Ptr& ctx);
        virtual ~KindChecking();

        virtual void visit(ASTNode*) override;

        virtual void visit(TypeDecl* tdecl);
        virtual void visit(ClassDecl* clss);

        virtual void visit(TypeMemberAccess* tdot);
        virtual void visit(TypeTuple* ttuple);
        virtual void visit(TypeConstructorCreation* typeconstructor);
        virtual void visit(TypeConstructorCall* tcall);
        virtual void visit(TypeIdentifier* tident);

    private:

        common::AbstractReporter& _rep;

        std::set<TypeDecl*> _visitedTypeDefs;
    };
}

}

#endif
