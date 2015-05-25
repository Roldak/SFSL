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
#include <set>
#include "../AST/Visitors/ASTVisitor.h"
#include "../AST/Symbols/SymbolResolver.h"

namespace sfsl {

namespace ast {

    /**
     * @brief
     */
    class TypeChecking : public ASTVisitor {
    public:

        TypeChecking(CompCtx_Ptr& ctx, const sym::SymbolResolver& res);
        virtual ~TypeChecking();

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

        type::Type* tryGetTypeOfSymbol(sym::Symbol* sym);
        type::Type* tryGetTypeOfField(ClassDecl* clss, const std::string& id, const type::SubstitutionTable& subtable);
        type::ProperType* applySubsitutions(type::ProperType* inner, type::ProperType* obj);
        type::ConstructorType* applySubsitutions(type::ConstructorType* inner, type::ProperType* obj);

        const sym::SymbolResolver& _res;
        common::AbstractReporter& _rep;

        std::set<DefineDecl*> _visitedDefs;
    };
}

}

#endif
