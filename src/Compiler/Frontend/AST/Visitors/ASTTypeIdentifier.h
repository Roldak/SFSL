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
    ASTTypeIdentifier(CompCtx_Ptr& ctx) : ASTVisitor(ctx), _res(false) {}

    virtual ~ASTTypeIdentifier() {}

    virtual void visit(ASTNode*) override {
        setIfSame<ASTNode>();
    }

    virtual void visit(Program*) override {
        setIfSame<Program>();
        setIfSame<ASTNode>();
    }

    virtual void visit(ModuleDecl*) override {
        setIfSame<ModuleDecl>();
        setIfSame<ASTNode>();
    }

    virtual void visit(TypeDecl*) override {
        setIfSame<TypeDecl>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(ClassDecl*) override {
        setIfSame<ClassDecl>();
        setIfSame<TypeExpression>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(DefineDecl*) override {
        setIfSame<DefineDecl>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(ProperTypeKindSpecifier*) override {
        setIfSame<ProperTypeKindSpecifier>();
        setIfSame<KindSpecifyingExpression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(TypeConstructorKindSpecifier*) override {
        setIfSame<TypeConstructorKindSpecifier>();
        setIfSame<KindSpecifyingExpression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(FunctionTypeDecl*) override {
        setIfSame<FunctionTypeDecl>();
        setIfSame<TypeExpression>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(TypeMemberAccess*) override {
        setIfSame<TypeMemberAccess>();
        setIfSame<TypeExpression>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(TypeTuple*) override {
        setIfSame<TypeTuple>();
        setIfSame<TypeExpression>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(TypeConstructorCreation*) override {
        setIfSame<TypeConstructorCreation>();
        setIfSame<TypeExpression>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(TypeConstructorCall*) override {
        setIfSame<TypeConstructorCall>();
        setIfSame<TypeExpression>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(TypeIdentifier*) override {
        setIfSame<TypeIdentifier>();
        setIfSame<TypeExpression>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(TypeToBeInferred*) override {
        setIfSame<TypeToBeInferred>();
        setIfSame<TypeExpression>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(TypeParameter*) override {
        setIfSame<TypeParameter>();
        setIfSame<TypeExpression>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(ExpressionStatement*) override {
        setIfSame<ExpressionStatement>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(AssignmentExpression*) override {
        setIfSame<AssignmentExpression>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(TypeSpecifier*) override {
        setIfSame<TypeSpecifier>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(Block*) override {
        setIfSame<Block>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(IfExpression*) override {
        setIfSame<IfExpression>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(MemberAccess*) override {
        setIfSame<MemberAccess>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(Tuple*) override {
        setIfSame<Tuple>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(FunctionCreation*) override {
        setIfSame<FunctionCreation>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(FunctionCall*) override {
        setIfSame<FunctionCall>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(Instantiation*) override {
        setIfSame<Instantiation>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(Identifier*) override {
        setIfSame<Identifier>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(This*) override {
        setIfSame<This>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(BoolLiteral*) override {
        setIfSame<BoolLiteral>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(IntLiteral*) override {
        setIfSame<IntLiteral>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(RealLiteral*) override {
        setIfSame<RealLiteral>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
    }

    virtual void visit(StringLiteral*) override {
        setIfSame<StringLiteral>();
        setIfSame<Expression>();
        setIfSame<ASTNode>();
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
        _res = _res | std::is_same<T, K>::value;
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

template<typename T, typename K>
/**
 * @brief Utility function that can dynamically test the type of
 * an AST node against the wanted type, and return itself as an instance
 * of the wanted type if the test is successful
 *
 * @param node The node for which to test the type
 * @param ctx The compilation context
 * @return itself under the wanted type if matches, otherwise nullptr
 */
T* getIfNodeOfType(K* node, CompCtx_Ptr& ctx) {
    if (isNodeOfType<T>(node, ctx)) {
        return static_cast<T*>(node);
    } else {
        return nullptr;
    }
}

}

}

#endif
