//
//  SubstitutionTable.cpp
//  SFSL
//
//  Created by Romain Beguet on 13.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "SubstitutionTable.h"
#include "Types.h"
#include <algorithm>

namespace sfsl {

namespace type {

namespace impl {

SubstitutionTable SubstitutionTable::Empty = type::SubstitutionTable();

SubstitutionTable::Substitution::Substitution()
    : varianceType(common::VAR_T_NONE), key(nullptr), value(nullptr) {

}

SubstitutionTable::Substitution::Substitution(common::VARIANCE_TYPE vt, Type* k, Type* v)
    : varianceType(vt), key(k), value(v) {

}

SubstitutionTable::SubstitutionComparator SubstitutionTable::Comparator;

SubstitutionTable::SubstitutionTable() {

}

SubstitutionTable::~SubstitutionTable() {

}

bool SubstitutionTable::equals(const SubstitutionTable& other) const {
    if (_subs.size() != other.size()) {
        return false;
    }

    for (size_t i = 0; i < _subs.size(); ++i) {
        if (_subs[i].key != other._subs[i].key ||
                !_subs[i].value->equals(other._subs[i].value)) {
            return false;
        }
    }

    return true;
}

bool SubstitutionTable::empty() const {
    return _subs.empty();
}

size_t SubstitutionTable::size() const {
    return _subs.size();
}

SubstitutionTable::iterator SubstitutionTable::insert(const SubstitutionTable::Substitution& p) {
    auto lb = std::lower_bound(begin(), end(), p, Comparator);
    return _subs.insert(lb, p);
}

void SubstitutionTable::insert(SubstitutionTable::const_iterator b, SubstitutionTable::const_iterator e) {
    std::for_each(b, e, [=](const Substitution& p) { insert(p); });
}

SubstitutionTable::iterator SubstitutionTable::begin() {
    return _subs.begin();
}

SubstitutionTable::const_iterator SubstitutionTable::begin() const {
    return _subs.begin();
}

SubstitutionTable::iterator SubstitutionTable::end() {
    return _subs.end();
}

SubstitutionTable::const_iterator SubstitutionTable::end() const {
    return _subs.end();
}

Type*& SubstitutionTable::operator [](Type* key) {
    iterator it = find(key);
    if (it != end()) {
        return it->value;
    } else {
        return insert(Substitution(common::VAR_T_NONE, key, nullptr))->value;
    }
}

SubstitutionTable::iterator SubstitutionTable::find(const Type* key) {
    for (size_t i = 0, e = _subs.size(); i < e; ++i) {
        if (_subs[i].key == key) {
            return begin() + i;
        }
    }
    return end();
}

SubstitutionTable::const_iterator SubstitutionTable::find(const Type* key) const {
    for (size_t i = 0, e = _subs.size(); i < e; ++i) {
        if (_subs[i].key == key) {
            return begin() + i;
        }
    }
    return end();
}

Type* SubstitutionTable::findSubstOrReturnMe(Type* toFind, bool* found) const {
    auto it = find(toFind);
    bool didFound = (it != end());

    if (found) {
        *found = didFound;
    }

    return didFound ? it->value : toFind;
}

bool SubstitutionTable::substituteAll(const SubstitutionTable& env) {
    bool matched = false;
    for (auto& pair : _subs) {
        bool tmp;
        pair.value = env.findSubstOrReturnMe(pair.value, &tmp);
        matched |= tmp;
    }
    return matched;
}

std::string SubstitutionTable::toString() const {
    return std::accumulate(begin(), end(), std::string("{"), [](const std::string& str, const SubstitutionTable::Substitution& sub) {
        return str + common::varianceTypeToString(sub.varianceType, true) + sub.key->toString() + " => " + sub.value->toString() + " ; ";
    }) + "}";
}

bool SubstitutionTable::SubstitutionComparator::operator ()(const SubstitutionTable::Substitution& a, const SubstitutionTable::Substitution& b) const {
    return a.key < b.key;
}

}

}

}
