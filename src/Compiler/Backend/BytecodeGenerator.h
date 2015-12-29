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
#include "UserDataAssignment.h"

namespace sfsl {

namespace bc {

using namespace ast;

class BytecodeGenerator : public out::CodeGenerator<BCInstruction*> {
public:
    BytecodeGenerator(CompCtx_Ptr& ctx, out::CodeGenOutput<BCInstruction*>& out);
    virtual ~BytecodeGenerator();

protected:

    BytecodeGenerator(CompCtx_Ptr& ctx, out::CodeGenOutput<BCInstruction*>& out, std::shared_ptr<out::Cursor*> constantPoolCursor);

    out::Cursor* Here() const;
    out::Cursor* End() const;
    void Seek(out::Cursor* cursor);

    Label* MakeLabel(const common::Positionnable& pos, const std::string& name);
    void BindLabel(Label* label);

    template<typename T, typename... Args>
    T* Emit(const common::Positionnable& pos, Args... args);

    template<typename T>
    T* Emit(T* instr);

    size_t getClassLoc(ast::ClassDecl* clss);
    size_t getDefLoc(sym::DefinitionSymbol* def);
    size_t getVarLoc(sym::VariableSymbol* var);

    std::shared_ptr<out::Cursor*> _constantPoolCursor;
};

/**
 * @brief Base class for visitors that generate code from the AST
 */
class DefaultBytecodeGenerator : public BytecodeGenerator {
public:

    DefaultBytecodeGenerator(CompCtx_Ptr& ctx, out::CodeGenOutput<BCInstruction*>& out);
    DefaultBytecodeGenerator(CompCtx_Ptr& ctx, out::CodeGenOutput<BCInstruction*>& out, std::shared_ptr<out::Cursor*> constantPoolCursor);
    virtual ~DefaultBytecodeGenerator();

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

    virtual void visit(AssignmentExpression* aex) override;
    virtual void visit(TypeSpecifier* tps) override;
    virtual void visit(Block* block) override;
    virtual void visit(IfExpression* ifexpr) override;
    virtual void visit(MemberAccess* dot) override;
    virtual void visit(Tuple* tuple) override;
    virtual void visit(FunctionCreation* func) override;
    virtual void visit(FunctionCall* call) override;
    virtual void visit(Instantiation* inst) override;
    virtual void visit(Identifier* ident) override;
    virtual void visit(This* ths) override;
    virtual void visit(BoolLitteral* boollit) override;
    virtual void visit(IntLitteral* intlit) override;
    virtual void visit(RealLitteral* reallit) override;

private:

    class AssignmentBytecodeGenerator : public BytecodeGenerator {
    public:
        AssignmentBytecodeGenerator(CompCtx_Ptr& ctx, out::CodeGenOutput<BCInstruction*>& out);
        AssignmentBytecodeGenerator(CompCtx_Ptr& ctx, out::CodeGenOutput<BCInstruction*>& out, std::shared_ptr<out::Cursor*> constantPoolCursor);
        virtual ~AssignmentBytecodeGenerator();

        virtual void visit(ASTNode*) override;

        virtual void visit(TypeSpecifier* tps) override;
        virtual void visit(MemberAccess* dot) override;
        virtual void visit(IfExpression* ifexpr) override;
        virtual void visit(Identifier* ident) override;
    };

    std::set<ClassDecl*> _visitedClasses;
};

}

}

#endif
