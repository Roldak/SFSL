//
//  SubstitutionTable.h
//  SFSL
//
//  Created by Romain Beguet on 13.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__SubstitutionTable__
#define __SFSL__SubstitutionTable__

#include <vector>
#include <map>
#include "../Common/Miscellaneous.h"

namespace sfsl {

namespace type {

class Type;

/**
 * @brief
 */
class Environment final {
public:
    struct Substitution {
        Substitution();
        Substitution(common::VARIANCE_TYPE vt, Type* k, Type* v);

        common::VARIANCE_TYPE varianceType;
        Type* key;
        Type* value;
    };

    typedef std::vector<Substitution>::iterator iterator;
    typedef std::vector<Substitution>::const_iterator const_iterator;

    Environment();
    ~Environment();

    bool empty() const;
    size_t size() const;

    iterator insert(const Substitution& p);
    void insert(const_iterator b, const_iterator e);

    iterator begin();
    const_iterator begin() const;

    iterator end();
    const_iterator end() const;

    Type*& operator [](Type* key);
    iterator find(const Type* key);
    const_iterator find(const Type* key) const;
    Type* findSubstOrReturnMe(Type* toFind, bool* found = nullptr) const;

    bool substituteAll(const Environment& env);

    std::string toString() const;

    static const Environment Empty;

private:

    struct SubstitutionComparator final {
        bool operator ()(const Substitution& a, const Substitution& b) const;
    };

    static SubstitutionComparator Comparator;

    std::vector<Substitution> _subs;
};

}

}

#endif
