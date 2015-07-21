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
#include "CodeGenOutput.h"

namespace sfsl {

namespace out {

    using namespace ast;

    /**
     * @brief
     */
    class CodeGenerator : public ASTVisitor {
    public:

        CodeGenerator(CompCtx_Ptr& ctx, CodeGenOutput<int>& out);
        virtual ~CodeGenerator();

        virtual void visit(ASTNode*) override;

        virtual void visit(Program* prog);

        virtual void visit(ModuleDecl* module);
        virtual void visit(TypeDecl* tdecl);
        virtual void visit(ClassDecl* clss);
        virtual void visit(DefineDecl* decl);

        virtual void visit(ProperTypeKindSpecifier* ptks);
        virtual void visit(TypeConstructorKindSpecifier* tcks);

        virtual void visit(TypeMemberAccess* tdot);
        virtual void visit(TypeTuple* ttuple);
        virtual void visit(TypeConstructorCreation* typeconstructor);
        virtual void visit(TypeConstructorCall* tcall);
        virtual void visit(TypeIdentifier* tident);
        virtual void visit(KindSpecifier* ks);

        virtual void visit(ExpressionStatement* exp);

        virtual void visit(BinaryExpression* bin);
        virtual void visit(AssignmentExpression* aex);
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

        CodeGenOutput<int>& _out;
    };
}

}

#endif
