//
//  KindExpressions.h
//  SFSL
//
//  Created by Romain Beguet on 14.05.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__KindExpressions__
#define __SFSL__KindExpressions__

#include <iostream>

#include "ASTNode.h"

#include "../../Kinds/Kinds.h"
#include "../../Common/Miscellaneous.h"
#include "../../../Common/ManageableUserData.h"

namespace sfsl {

namespace ast {

class TypeExpression;
class TypeIdentifier;

/**
 * @brief A superclass that represents a kind expression.
 * Cannot be constructed. This class is there just to provide
 * a lower upper bound for all the kind expressions than ASTNode
 */
class KindSpecifyingExpression : public ASTNode, public kind::Kinded, public common::HasManageableUserdata {
public:

    virtual ~KindSpecifyingExpression();

    SFSL_AST_ON_VISIT_H
};

/**
 * @brief A kind expression representing the proper kind: The expression `*`
 */
class ProperTypeKindSpecifier : public KindSpecifyingExpression {
public:

    ProperTypeKindSpecifier(TypeExpression* lowerBound = nullptr, TypeExpression* upperBound = nullptr);
    virtual ~ProperTypeKindSpecifier();

    SFSL_AST_ON_VISIT_H

    TypeExpression* getLowerBoundExpr() const;
    TypeExpression* getUpperBoundExpr() const;

private:

    TypeExpression* _lb;
    TypeExpression* _ub;
};

/**
 * @brief A kind expression representing the type constructor kind, e.g. `*->*`, `[in *, out *]->*` etc.
 */
class TypeConstructorKindSpecifier: public KindSpecifyingExpression {
public:
    struct Parameter {
        Parameter();
        Parameter(common::VARIANCE_TYPE vt, KindSpecifyingExpression* expr);

        operator bool() const;

        common::VARIANCE_TYPE varianceType;
        KindSpecifyingExpression* kindExpr;
    };

    TypeConstructorKindSpecifier(const std::vector<Parameter>& args, KindSpecifyingExpression* ret = nullptr);
    virtual ~TypeConstructorKindSpecifier();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The sequence of kind expressions that compose the type constructor specifier
     */
    const std::vector<Parameter>& getArgs() const;

    /**
     * @return The ret kind of the type constructor
     */
    KindSpecifyingExpression* getRet() const;

    /**
     * @param expr The new kind specifying expression specifying the return kind
     * of this type constructor specifier
     */
    void setRet(KindSpecifyingExpression* expr);

private:

    std::vector<Parameter> _args;
    KindSpecifyingExpression* _ret;
};


}

}

#endif
