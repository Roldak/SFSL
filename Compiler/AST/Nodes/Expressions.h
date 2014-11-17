//
//  Expressions.h
//  SFSL
//
//  Created by Romain Beguet on 17.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Expressions__
#define __SFSL__Expressions__

#include <iostream>
#include "../../../Utils/Utils.h"
#include "ASTNode.h"

namespace sfsl {

namespace ast {

class Expression : public ASTNode {
public:

    Expression();
    virtual ~Expression();

    SFSL_AST_ON_VISIT_H

};

class Identifier : public Expression {
public:

    Identifier(const std::string& name);
    virtual ~Identifier();

    SFSL_AST_ON_VISIT_H

    const std::string& getValue() const;

private:

    const std::string _name;

};

class IntLitteral : public Expression {
public:

    IntLitteral(const sfsl_int_t value);
    virtual ~IntLitteral();

    SFSL_AST_ON_VISIT_H

    sfsl_int_t getValue() const;

private:

    const sfsl_int_t _value;
};

class RealLitteral : public Expression {
public:

    RealLitteral(const sfsl_real_t value);
    virtual ~RealLitteral();

    SFSL_AST_ON_VISIT_H

    sfsl_real_t getValue() const;

private:

    const sfsl_real_t _value;
};

}

}

#endif
