//
//  ASTTypeIdentifier.h
//  SFSL
//
//  Created by Romain Beguet on 29.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ASTTypeIdentifier__
#define __SFSL__ASTTypeIdentifier__

#include <iostream>
#include "ASTVisitor.h"

namespace sfsl {

namespace ast {

template<typename T>
/**
 * @brief A visitor that dynamically tests the type of the node which is visited by it
 */
class ASTTypeIdentifier : public ASTVisitor {
public:

    /**
     * @brief Creates an ASTTypeIdentifier
     * @param ctx the compilation context that will be used throughout the visits
     */
    ASTTypeIdentifier(CompCtx_Ptr& ctx) : ASTVisitor(ctx) {}

    virtual ~ASTTypeIdentifier() {}

    virtual void visit(ASTNode*) override {
        setIfSame<ASTNode>();
    }

    virtual void visit(Program*) override {
        setIfSame<Program>();
    }

    virtual void visit(ModuleDecl*) override {
        setIfSame<ModuleDecl>();
    }

    virtual void visit(TypeDecl*) override {
        setIfSame<TypeDecl>();
    }

    virtual void visit(ClassDecl*) override {
        setIfSame<ClassDecl>();
    }

    virtual void visit(DefineDecl*) override {
        setIfSame<DefineDecl>();
    }

    virtual void visit(ProperTypeKindSpecifier*) override {
        setIfSame<ProperTypeKindSpecifier>();
    }

    virtual void visit(TypeConstructorKindSpecifier*) override {
        setIfSame<TypeConstructorKindSpecifier>();
    }

    virtual void visit(FunctionTypeDecl*) override {
        setIfSame<FunctionTypeDecl>();
    }

    virtual void visit(TypeMemberAccess*) override {
        setIfSame<TypeMemberAccess>();
    }

    virtual void visit(TypeTuple*) override {
        setIfSame<TypeTuple>();
    }

    virtual void visit(TypeConstructorCreation*) override {
        setIfSame<TypeConstructorCreation>();
    }

    virtual void visit(TypeConstructorCall*) override {
        setIfSame<TypeConstructorCall>();
    }

    virtual void visit(TypeIdentifier*) override {
        setIfSame<TypeIdentifier>();
    }

    virtual void visit(TypeToBeInferred*) override {
        setIfSame<TypeToBeInferred>();
    }

    virtual void visit(KindSpecifier*) override {
        setIfSame<KindSpecifier>();
    }

    virtual void visit(ExpressionStatement*) override {
        setIfSame<ExpressionStatement>();
    }

    virtual void visit(AssignmentExpression*) override {
        setIfSame<AssignmentExpression>();
    }

    virtual void visit(TypeSpecifier*) override {
        setIfSame<TypeSpecifier>();
    }

    virtual void visit(Block*) override {
        setIfSame<Block>();
    }

    virtual void visit(IfExpression*) override {
        setIfSame<IfExpression>();
    }

    virtual void visit(MemberAccess*) override {
        setIfSame<MemberAccess>();
    }

    virtual void visit(Tuple*) override {
        setIfSame<Tuple>();
    }

    virtual void visit(FunctionCreation*) override {
        setIfSame<FunctionCreation>();
    }

    virtual void visit(FunctionCall*) override {
        setIfSame<FunctionCall>();
    }

    virtual void visit(Identifier*) override {
        setIfSame<Identifier>();
    }

    virtual void visit(This*) override {
        setIfSame<This>();
    }

    virtual void visit(BoolLitteral*) override {
        setIfSame<BoolLitteral>();
    }

    virtual void visit(IntLitteral*) override {
        setIfSame<IntLitteral>();
    }

    virtual void visit(RealLitteral*) override {
        setIfSame<RealLitteral>();
    }

    virtual void visit(StringLitteral*) override {
        setIfSame<StringLitteral>();
    }

    /**
     * @return True if the node is well of type T
     */
    bool matches() const {
        return _res;
    }

protected:

    template<typename K>
    void setIfSame() {
        _res = std::is_same<T, K>::value;
    }

    bool _res;
};

template<typename T>
/**
 * @brief Utility function that can dynamically test the type of
 * an AST node against the wanted type
 *
 * @param node The node for which to test the type
 * @param ctx The compilation context
 * @return True if the type matches, else false
 */
bool isNodeOfType(ASTNode* node, CompCtx_Ptr& ctx) {
    ASTTypeIdentifier<T> identifier(ctx);
    node->onVisit(&identifier);
    return identifier.matches();
}

}

}

#endif
