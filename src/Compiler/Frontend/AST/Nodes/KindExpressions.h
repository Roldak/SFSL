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
#include "../Utils/Miscellaneous.h"

namespace sfsl {

namespace ast {

class TypeIdentifier;

/**
 * @brief A superclass that represents a kind expression.
 * Cannot be constructed. This class is there just to provide
 * a lower upper bound for all the kind expressions than ASTNode
 */
class KindSpecifyingExpression : public ASTNode, public kind::Kinded {
public:

    virtual ~KindSpecifyingExpression();

    SFSL_AST_ON_VISIT_H
};

/**
 * @brief A kind expression representing the proper kind: The expression `*`
 */
class ProperTypeKindSpecifier : public KindSpecifyingExpression {
public:

    ProperTypeKindSpecifier();
    virtual ~ProperTypeKindSpecifier();

    SFSL_AST_ON_VISIT_H

private:
};

/**
 * @brief A kind expression representing the type constructor kind, e.g. `*->*`, `[in *, out *]->*` etc.
 */
class TypeConstructorKindSpecifier: public KindSpecifyingExpression {
public:
    struct Parameter {
        Parameter();
        Parameter(VARIANCE_TYPE vt, KindSpecifyingExpression* expr);

        operator bool() const;

        VARIANCE_TYPE varianceType;
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
