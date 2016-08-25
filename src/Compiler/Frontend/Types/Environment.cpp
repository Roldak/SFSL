//
//  SubstitutionTable.cpp
//  SFSL
//
//  Created by Romain Beguet on 13.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "Environment.h"
#include "Types.h"
#include <algorithm>

namespace sfsl {

namespace type {

Environment Environment::Empty = type::Environment();

Environment::Substitution::Substitution()
    : varianceType(common::VAR_T_NONE), key(nullptr), value(nullptr) {

}

Environment::Substitution::Substitution(common::VARIANCE_TYPE vt, Type* k, Type* v)
    : varianceType(vt), key(k), value(v) {

}

Environment::SubstitutionComparator Environment::Comparator;

Environment::Environment() {

}

Environment::~Environment() {

}

bool Environment::empty() const {
    return _subs.empty();
}

size_t Environment::size() const {
    return _subs.size();
}

Environment::iterator Environment::insert(const Environment::Substitution& p) {
    auto lb = std::lower_bound(begin(), end(), p, Comparator);
    return _subs.insert(lb, p);
}

void Environment::insert(Environment::const_iterator b, Environment::const_iterator e) {
    std::for_each(b, e, [=](const Substitution& p) { insert(p); });
}

Environment::iterator Environment::begin() {
    return _subs.begin();
}

Environment::const_iterator Environment::begin() const {
    return _subs.begin();
}

Environment::iterator Environment::end() {
    return _subs.end();
}

Environment::const_iterator Environment::end() const {
    return _subs.end();
}

Type*& Environment::operator [](Type* key) {
    iterator it = find(key);
    if (it != end()) {
        return it->value;
    } else {
        return insert(Substitution(common::VAR_T_NONE, key, nullptr))->value;
    }
}

Environment::iterator Environment::find(const Type* key) {
    for (size_t i = 0, e = _subs.size(); i < e; ++i) {
        if (_subs[i].key == key) {
            return begin() + i;
        }
    }
    return end();
}

Environment::const_iterator Environment::find(const Type* key) const {
    for (size_t i = 0, e = _subs.size(); i < e; ++i) {
        if (_subs[i].key == key) {
            return begin() + i;
        }
    }
    return end();
}

Type* Environment::findSubstOrReturnMe(Type* toFind, bool* found) const {
    auto it = find(toFind);
    bool didFound = (it != end());

    if (found) {
        *found = didFound;
    }

    return didFound ? it->value : toFind;
}

bool Environment::substituteAll(const Environment& env) {
    bool matched = false;
    for (auto& pair : _subs) {
        bool tmp;
        pair.value = env.findSubstOrReturnMe(pair.value, &tmp);
        matched |= tmp;
    }
    return matched;
}

std::string Environment::toString() const {
    return std::accumulate(begin(), end(), std::string("{"), [](const std::string& str, const Environment::Substitution& sub) {
        return str + common::varianceTypeToString(sub.varianceType, true) + sub.key->toString() + " => " + sub.value->toString() + " ; ";
    }) + "}";
}

bool Environment::SubstitutionComparator::operator ()(const Environment::Substitution& a, const Environment::Substitution& b) const {
    return a.key < b.key;
}

}

}
