//
//  BytecodeGenerator.h
//  SFSL
//
//  Created by Romain Beguet on 22.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__BytecodeGenerator__
#define __SFSL__BytecodeGenerator__

#include <iostream>
#include "CodeGen/CodeGenerator.h"
#include "Bytecode/Bytecode.h"

namespace sfsl {

namespace bc {

    using namespace ast;

    /**
     * @brief Base class for visitors that generate code from the AST
     */
    class BytecodeGenerator : public out::CodeGenerator<BCInstruction*> {
    public:

        BytecodeGenerator(CompCtx_Ptr& ctx, out::CodeGenOutput<BCInstruction*>& out);
        virtual ~BytecodeGenerator();

        virtual void visit(ASTNode*) override;

        virtual void visit(Program* prog) override;

        virtual void visit(ModuleDecl* module) override;
        virtual void visit(TypeDecl* tdecl) override;
        virtual void visit(ClassDecl* clss) override;
        virtual void visit(DefineDecl* decl) override;

        virtual void visit(ProperTypeKindSpecifier* ptks) override;
        virtual void visit(TypeConstructorKindSpecifier* tcks) override;

        virtual void visit(TypeMemberAccess* tdot) override;
        virtual void visit(TypeTuple* ttuple) override;
        virtual void visit(TypeConstructorCreation* typeconstructor) override;
        virtual void visit(TypeConstructorCall* tcall) override;
        virtual void visit(TypeIdentifier* tident) override;
        virtual void visit(KindSpecifier* ks) override;

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

        out::Cursor* Here() const;
        out::Cursor* End() const;
        void Seek(out::Cursor* cursor);

        template<typename T, typename... Args>
        void Emit(const common::Positionnable& pos, Args... args);

        size_t _currentVarCount;

    };

    class VarUserData final : public common::MemoryManageable {
    public:
        VarUserData(size_t loc);
        virtual ~VarUserData();

        size_t getVarLoc() const;

    private:

        size_t _loc;
    };
}

}

#endif
