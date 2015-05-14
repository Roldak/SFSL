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
#include "../../Types/Kinds/Kinds.h"

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
 * @brief Represents a tuple
 */
class ProperTypeKindSpecifier : public KindSpecifyingExpression {
public:

    ProperTypeKindSpecifier();
    virtual ~ProperTypeKindSpecifier();

    SFSL_AST_ON_VISIT_H

private:
};

/**
 * @brief Represents a type constructor creation, e.g. `[T] => class { x: T; }`
 */
class TypeConstructorKindSpecifier: public KindSpecifyingExpression {
public:

    TypeConstructorKindSpecifier(const std::vector<KindSpecifyingExpression*>& args, KindSpecifyingExpression* ret = nullptr);
    virtual ~TypeConstructorKindSpecifier();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The sequence of kind expressions that compose the type constructor specifier
     */
    const std::vector<KindSpecifyingExpression*>& getArgs() const;

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

    std::vector<KindSpecifyingExpression*> _args;
    KindSpecifyingExpression* _ret;
};


}

}

#endif
