//
//  ASTTypeCreator.h
//  SFSL
//
//  Created by Romain Beguet on 31.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ASTTypeCreator__
#define __SFSL__ASTTypeCreator__

#include <iostream>
#include <set>
#include "ASTExplicitVisitor.h"
#include "../../Symbols/SymbolResolver.h"
#include "../../Types/Types.h"

namespace sfsl {

namespace ast {

/**
 * @brief A visitor that can generate a type from an ASTNode
 */
class ASTTypeCreator : public ASTExplicitVisitor {
public:

    /**
     * @brief Creates an ASTTypeCreator
     * @param ctx the compilation context that will be used throughout the visits
     */
    ASTTypeCreator(CompCtx_Ptr& ctx, const std::vector<type::Type*>& args);

    virtual ~ASTTypeCreator();

    virtual void visit(ASTNode* node) override;

    virtual void visit(ClassDecl* clss) override;
    virtual void visit(FunctionTypeDecl* ftdecl) override;
    virtual void visit(TypeConstructorCreation* typeconstructor) override;
    virtual void visit(TypeConstructorCall* tcall) override;

    virtual void visit(TypeMemberAccess* mac) override;
    virtual void visit(TypeIdentifier* ident) override;

    /**
     * @return The type created by the ASTTypeCreator
     */
    type::Type* getCreatedType() const;

    /**
     * @brief Creates a type from an ASTNode, if the node corresponds
     * to a valid syntax of a type node.
     *
     * @param node The node from which to create the type
     * @param ctx The compilation context
     * @return The generated type
     */
    static type::Type* createType(ASTNode* node, CompCtx_Ptr& ctx);
    static type::Type* evalTypeConstructor(type::TypeConstructorType* ctr, CompCtx_Ptr& ctx, const std::vector<type::Type*>& args);

protected:

    void createTypeFromSymbolic(sym::Symbolic<sym::Symbol>* symbolic, common::Positionnable& pos);
    type::SubstitutionTable buildSubstitutionTableFromTypeParametrizable(type::TypeParametrizable* param);

    type::Type* _created;

    const std::vector<type::Type*>& _args;

    std::set<sym::TypeSymbol*> _visitedTypes;
};

/**
 * @brief Evaluates the type of the TypeSymbol and returns the ClassDecl
 * associated to the ProperType in case it evaluated to ProperType,
 * otherwise returns nullptr
 *
 * @param sym The TypeSymbol from which to get the ClassDecl
 * @param ctx The compilation context
 * @return The ClassDecl is found, otherwise nullptr
 */
inline ast::ClassDecl* getClassDeclFromTypeSymbol(sym::TypeSymbol* sym, CompCtx_Ptr& ctx) {
    if (type::Type* t = ASTTypeCreator::createType(sym->getTypeDecl()->getExpression(), ctx)) {
        if (type::ProperType* o = type::getIf<type::ProperType>(t)) {
            return o->getClass();
        }
    }
    return nullptr;
}

}

}

#endif
