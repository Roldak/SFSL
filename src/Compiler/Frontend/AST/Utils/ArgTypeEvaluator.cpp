//
//  ArgTypeEvaluator.cpp
//  SFSL
//
//  Created by Romain Beguet on 24.08.16.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "ArgTypeEvaluator.h"
#include "../../Analyser/TypeChecking.h"

namespace sfsl {

namespace ast {

// ARG TYPE EVALUATOR

ArgTypeEvaluator::ArgTypeEvaluator(TypeChecking* checker, const std::vector<Expression*>& argExprs)
    : _checker(checker), _argExprs(argExprs), _evaluated(argExprs.size(), false) {

}

type::Type* ArgTypeEvaluator::at(size_t i) {
    if (!_evaluated[i]) {
        _argExprs[i]->onVisit(_checker);
        _evaluated[i] = true;
    }

    return _argExprs[i]->type();
}

size_t ArgTypeEvaluator::size() const {
    return _argExprs.size();
}

void ArgTypeEvaluator::evalAll(const std::vector<type::Type*>& expectedTypes) {
    TypeChecking::ExpectedInfo save = _checker->_expectedInfo;

    for (size_t i = 0; i < _argExprs.size(); ++i) {
        if (!_evaluated[i]) {
            if (expectedTypes.size() > i) {
                _checker->_expectedInfo.node = _argExprs[i];
                _checker->_expectedInfo.ret = expectedTypes[i];
            }

            _argExprs[i]->onVisit(_checker);
            _evaluated[i] = true;
        }
    }

    _checker->_expectedInfo = save;
}

}

}
