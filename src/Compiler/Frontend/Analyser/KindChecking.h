//
//  KindChecking.h
//  SFSL
//
//  Created by Romain Beguet on 12.05.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__KindChecking__
#define __SFSL__KindChecking__

#include <iostream>
#include <set>
#include "../AST/Visitors/ASTImplicitVisitor.h"
#include "../Symbols/SymbolResolver.h"

namespace sfsl {

namespace ast {

/**
 * @brief
 */
class KindChecking : public ASTImplicitVisitor {
public:

    KindChecking(CompCtx_Ptr& ctx);
    virtual ~KindChecking();

    virtual void visit(ASTNode*) override;

    virtual void visit(Program* prog) override;

    virtual void visit(TypeDecl* tdecl) override;
    virtual void visit(ClassDecl* clss) override;
    virtual void visit(DefineDecl* decl) override;

    virtual void visit(FunctionTypeDecl* ftdecl) override;
    virtual void visit(TypeMemberAccess* tdot) override;
    virtual void visit(TypeTuple* ttuple) override;
    virtual void visit(TypeConstructorCreation* typeconstructor) override;
    virtual void visit(TypeConstructorCall* tcall) override;
    virtual void visit(TypeIdentifier* tident) override;
    virtual void visit(TypeToBeInferred* tbi) override;
    virtual void visit(TypeParameter* tparam) override;

    virtual void visit(TypeSpecifier* ts) override;
    virtual void visit(Instantiation* inst) override;

    static bool kindCheckArgumentSubstitution(const std::vector<kind::Kind*>& parametersKinds,
                                              const std::vector<TypeExpression*>& arguments,
                                              const common::Positionnable& callPos, CompCtx_Ptr& ctx);

    static bool kindCheckWithBoundsArgumentSubstitution(const std::vector<kind::Kind*>& parametersKinds,
                                                        const std::vector<TypeExpression*>& arguments,
                                                        const std::vector<type::Type*>& createdArguments,
                                                        const common::Positionnable& callPos,
                                                        const type::Environment& env, CompCtx_Ptr& ctx,
                                                        bool reportErrors = true);

private:

    kind::ProperKind* createProperKindWithBounds(TypeExpression* lb, TypeExpression* ub);

    void visitDeferredExpressions();

    template<typename T>
    void trySetKindOfSymbolic(T* symbolic);

    kind::Kind* tryGetKindOfSymbol(sym::Symbol* sym);

    common::AbstractReporter& _rep;

    std::set<TypeDecl*> _visitedTypeDefs;
    std::set<TypeExpression*> _deferredExpressions;
    bool _mustDefer;

    bool _insideMemberAccess;
};

}

}

#endif
