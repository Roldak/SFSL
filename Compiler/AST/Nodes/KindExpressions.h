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
class KindExpression : public ASTNode, public kind::Kinded {
public:

    virtual ~KindExpression();

    SFSL_AST_ON_VISIT_H
};

/**
 * @brief Represents a tuple
 */
class ProperTypeKindSpecifier : public KindExpression {
public:

    ProperTypeKindSpecifier();
    virtual ~ProperTypeKindSpecifier();

    SFSL_AST_ON_VISIT_H

private:
};

/**
 * @brief Represents a type constructor creation, e.g. `[T] => class { x: T; }`
 */
class TypeConstructorKindSpecifier: public KindExpression {
public:

    TypeConstructorKindSpecifier(const std::vector<KindExpression*>& args, KindExpression* ret);
    virtual ~TypeConstructorKindSpecifier();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The sequence of kind expressions that compose the type constructor specifier
     */
    const std::vector<KindExpression*>& getArgs() const;

    /**
     * @return The ret kind of the type constructor
     */
    KindExpression* getRet() const;

private:

    std::vector<KindExpression*> _args;
    KindExpression* _ret;
};


}

}

#endif
