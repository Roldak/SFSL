//
//  SubstitutionTable.cpp
//  SFSL
//
//  Created by Romain Beguet on 13.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "SubstitutionTable.h"
#include <algorithm>

namespace sfsl {

namespace type {

namespace impl {

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

bool SubstitutionTable::empty() const {
    return _subs.empty();
}

SubstitutionTable::iterator SubstitutionTable::insert(const SubstitutionTable::Substitution& p) {
    auto lb = std::lower_bound(begin(), end(), p, Comparator);
    return _subs.insert(lb, p);
}

void SubstitutionTable::insert(SubstitutionTable::const_iterator b, SubstitutionTable::const_iterator e) {
    std::for_each(b, e, [=](const Substitution& p) { insert(p); });
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

size_t SubstitutionTable::size() const {
    return _subs.size();
}

bool SubstitutionTable::SubstitutionComparator::operator ()(const SubstitutionTable::Substitution& a, const SubstitutionTable::Substitution& b) const {
    return a.key < b.key;
}

}

}

}
