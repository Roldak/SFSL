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
    ASTTypeCreator(CompCtx_Ptr& ctx, const std::vector<type::Type*>& args, bool allowFunctionConstructors);

    virtual ~ASTTypeCreator();

    virtual void visit(ASTNode* node) override;

    virtual void visit(ClassDecl* clss) override;
    virtual void visit(FunctionTypeDecl* ftdecl) override;
    virtual void visit(TypeConstructorCreation* typeconstructor) override;
    virtual void visit(TypeConstructorCall* tcall) override;

    virtual void visit(TypeMemberAccess* mac) override;
    virtual void visit(TypeIdentifier* ident) override;
    virtual void visit(Identifier* ident) override;

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
    static type::Type* createType(ASTNode* node, CompCtx_Ptr& ctx, bool allowFunctionConstructors = false);

    /**
     * @brief Evaluates the given type constructor with the given arguments
     * and returns its result.
     *
     * @param ctr The type constructor to evaluate
     * @param args The arguments to feed the type constructor with
     * @param ctx The compilation context
     * @return The type that was created after evaluating the type constructor
     * (or the `not yet defined` type in case of failure)
     */
    static type::Type* evalTypeConstructor(type::TypeConstructorType* ctr, const std::vector<type::Type*>& args, CompCtx_Ptr& ctx);

    /**
     * @brief Evaluates the given function constructor with the given arguments
     * and returns its result.
     *
     * @param fc The function constructor to evaluate (a FunctionType or a MethodType)
     * @param args The arguments to feed the function constructors with
     * @param callPos the position of the call
     * @param ctx The compilation context
     * @return The function type that was created after evaluating the type constructor
     * (or the `not yet defined` type in case of failure)
     */
    static type::Type* evalFunctionConstructor(type::Type* fc, const std::vector<TypeExpression*>& args,
                                               const common::Positionnable& callPos, CompCtx_Ptr& ctx);

    /**
     * @brief Creates a substitution table mapping the types of the parameters to the types of the arguments
     * @param typeParameters The expressions representing the type parameters (for example, TypeIdentifiers or KindSpecifiers)
     * @param args The corresponding vector of type arguments
     * @param ctx The compilation context
     * @return The substitution table that was created
     */
    static type::SubstitutionTable buildSubstitutionTableFromTypeParameterInstantiation(
            const std::vector<TypeExpression*> params, const std::vector<type::Type*>& args, CompCtx_Ptr& ctx);

protected:

    void createTypeFromSymbolic(sym::Symbolic<sym::Symbol>* symbolic, common::Positionnable& pos);
    type::SubstitutionTable buildSubstitutionTableFromTypeParametrizable(type::TypeParametrizable* param);

    type::Type* _created;

    const std::vector<type::Type*>& _args;

    bool _allowFunctionConstructors;

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
        if (type::ProperType* o = type::getIf<type::ProperType>(t->applyTCCallsOnly(ctx))) {
            return o->getClass();
        }
    }
    return nullptr;
}

}

}

#endif
