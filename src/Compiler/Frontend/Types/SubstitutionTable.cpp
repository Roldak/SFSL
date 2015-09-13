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
        return it->second;
    } else {
        return insert(std::make_pair(key, nullptr))->second;
    }
}

SubstitutionTable::iterator SubstitutionTable::find(const Type* key) {
    for (size_t i = 0; i < _subs.size(); ++i) {
        if (_subs[i].first == key) {
            return begin() + i;
        }
    }
    return end();
}

SubstitutionTable::const_iterator SubstitutionTable::find(const Type* key) const {
    for (size_t i = 0; i < _subs.size(); ++i) {
        if (_subs[i].first == key) {
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

bool SubstitutionTable::SubstitutionComparator::operator ()(const SubstitutionTable::Substitution& a, const SubstitutionTable::Substitution& b) const {
    return a.first < b.first;
}

}

}

}
