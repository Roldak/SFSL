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

namespace impl {

/**
 * @brief
 */
class SubstitutionTable final {
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

    SubstitutionTable();
    ~SubstitutionTable();

    bool equals(const SubstitutionTable& other) const;
    bool empty() const;

    iterator insert(const Substitution& p);
    void insert(const_iterator b, const_iterator e);

    Type*& operator [](Type* key);

    iterator find(const Type* key);
    const_iterator find(const Type* key) const;

    iterator begin();
    const_iterator begin() const;

    iterator end();
    const_iterator end() const;

    size_t size() const;

    static SubstitutionTable Empty;

private:

    struct SubstitutionComparator final {
        bool operator ()(const Substitution& a, const Substitution& b) const;
    };

    static SubstitutionComparator Comparator;

    std::vector<Substitution> _subs;
};

}

//typedef std::map<Type*, Type*> SubstitutionTable;
typedef impl::SubstitutionTable SubstitutionTable;

}

}

#endif
