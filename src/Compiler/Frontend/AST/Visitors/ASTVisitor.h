//
//  ASTImplicitVisitor.h
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ASTVisitor__
#define __SFSL__ASTVisitor__

#include <iostream>
#include "../Nodes/Program.h"
#include "../../../Common/CompilationContext.h"

#define OLD(memberName) __old##memberName
#define SAVE_MEMBER(memberName) auto OLD(memberName) = memberName;
#define SAVE_MEMBER_AND_SET(memberName, value) \
    SAVE_MEMBER(memberName) \
    memberName = value;
#define RESTORE_MEMBER(memberName) memberName = OLD(memberName);

#define TRY_INSERT(colName, value) (colName.insert(value).second)

namespace sfsl {

namespace ast {

/**
 * @brief An abstract class representing a visitor of ASTNodes.
 */
class ASTVisitor {
public:

    /**
     * @brief Creates an ASTVisitor
     * @param ctx the compilation context that will be used throughout the visits
     */
    ASTVisitor(CompCtx_Ptr& ctx);

    virtual ~ASTVisitor();

    /**
     * @brief Should never be called.
     * @param node The visited node
     */
    virtual void visit(ASTNode* node) = 0;

    virtual void visit(Program* prog) = 0;

    virtual void visit(ModuleDecl* module) = 0;
    virtual void visit(TypeDecl* tdecl) = 0;
    virtual void visit(ClassDecl* clss) = 0;
    virtual void visit(DefineDecl* decl) = 0;

    virtual void visit(ProperTypeKindSpecifier* ptks) = 0;
    virtual void visit(TypeConstructorKindSpecifier* tcks) = 0;

    virtual void visit(FunctionTypeDecl* ftdecl) = 0;
    virtual void visit(TypeMemberAccess* tdot) = 0;
    virtual void visit(TypeTuple* ttuple) = 0;
    virtual void visit(TypeConstructorCreation* typeconstructor) = 0;
    virtual void visit(TypeConstructorCall* tcall) = 0;
    virtual void visit(TypeIdentifier* tident) = 0;
    virtual void visit(TypeToBeInferred* tbi) = 0;
    virtual void visit(TypeParameter* tparam) = 0;

    virtual void visit(ExpressionStatement* exp) = 0;

    virtual void visit(AssignmentExpression* aex) = 0;
    virtual void visit(TypeSpecifier* tps) = 0;
    virtual void visit(Block* block) = 0;
    virtual void visit(IfExpression* ifexpr) = 0;
    virtual void visit(MemberAccess* dot) = 0;
    virtual void visit(Tuple* tuple) = 0;
    virtual void visit(FunctionCreation* func) = 0;
    virtual void visit(FunctionCall* call) = 0;
    virtual void visit(Instantiation* inst) = 0;
    virtual void visit(Identifier* ident) = 0;
    virtual void visit(This* ths) = 0;
    virtual void visit(BoolLiteral* boollit) = 0;
    virtual void visit(IntLiteral* intlit) = 0;
    virtual void visit(RealLiteral* reallit) = 0;
    virtual void visit(StringLiteral* strlit) = 0;

protected:

    CompCtx_Ptr _ctx;
    common::AbstractMemoryManager& _mngr;
};

}

}

#endif
