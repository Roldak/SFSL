//
//  ArgTypeEvaluator.h
//  SFSL
//
//  Created by Romain Beguet on 24.08.16.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ArgTypeEvaluator__
#define __SFSL__ArgTypeEvaluator__

#include <iostream>
#include <vector>

namespace sfsl {

namespace type {
class Type;
}

namespace ast {

class Expression;
class TypeChecking;

class ArgTypeEvaluator final {
public:
    ArgTypeEvaluator(TypeChecking* checker, const std::vector<Expression*>& argExprs);

    type::Type* at(size_t i, type::Type* expectedType = nullptr);
    size_t size() const;

    void evalAll(const std::vector<type::Type*>& expectedTypes);

private:

    void visit(size_t i, type::Type* expectedType = nullptr);

    TypeChecking* _checker;
    std::vector<Expression*> _argExprs;
    std::vector<bool> _evaluated;
};

}

}

#endif
