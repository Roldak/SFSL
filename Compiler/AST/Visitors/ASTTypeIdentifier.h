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
    ASTTypeIdentifier(std::shared_ptr<common::CompilationContext>& ctx) : ASTVisitor(ctx) {}

    virtual ~ASTTypeIdentifier() {}

    virtual void visit(ASTNode*) {
        setIfSame<ASTNode>();
    }

    virtual void visit(Program*) {
        setIfSame<Program>();
    }

    virtual void visit(ModuleDecl*) {
        setIfSame<ModuleDecl>();
    }

    virtual void visit(ClassDecl*) {
        setIfSame<ClassDecl>();
    }

    virtual void visit(DefineDecl*) {
        setIfSame<DefineDecl>();
    }

    virtual void visit(ExpressionStatement*) {
        setIfSame<ExpressionStatement>();
    }

    virtual void visit(BinaryExpression*) {
        setIfSame<BinaryExpression>();
    }

    virtual void visit(TypeSpecifier*) {
        setIfSame<TypeSpecifier>();
    }

    virtual void visit(Block*) {
        setIfSame<Block>();
    }

    virtual void visit(IfExpression*) {
        setIfSame<IfExpression>();
    }

    virtual void visit(MemberAccess*) {
        setIfSame<MemberAccess>();
    }

    virtual void visit(Tuple*) {
        setIfSame<Tuple>();
    }

    virtual void visit(FunctionCreation*) {
        setIfSame<FunctionCreation>();
    }

    virtual void visit(FunctionCall*) {
        setIfSame<FunctionCall>();
    }

    virtual void visit(Identifier*) {
        setIfSame<Identifier>();
    }

    virtual void visit(IntLitteral*) {
        setIfSame<IntLitteral>();
    }

    virtual void visit(RealLitteral*) {
        setIfSame<RealLitteral>();
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
bool isNodeOfType(ASTNode* node, std::shared_ptr<common::CompilationContext>& ctx) {
    ASTTypeIdentifier<T> identifier(ctx);
    node->onVisit(&identifier);
    return identifier.matches();
}

}

}

#endif
