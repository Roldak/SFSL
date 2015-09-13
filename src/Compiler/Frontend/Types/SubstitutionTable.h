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

namespace sfsl {

namespace type {

class Type;

namespace impl {

/**
 * @brief
 */
class SubstitutionTable final {
public:
    typedef std::pair<Type*, Type*> Substitution;
    typedef std::vector<Substitution>::iterator iterator;
    typedef std::vector<Substitution>::const_iterator const_iterator;

    SubstitutionTable();
    ~SubstitutionTable();

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
