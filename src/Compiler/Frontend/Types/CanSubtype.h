//
//  CanSubtype.h
//  SFSL
//
//  Created by Romain Beguet on 24.08.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__CanSubtype__
#define __SFSL__CanSubtype__

#include <algorithm>
#include <vector>

namespace sfsl {

namespace type {

template<typename ParentType, template<typename, typename> class Container>
class CanSubtypeBase {
    typedef Container<ParentType*, std::allocator<ParentType*>> ParentTypeContainer;

public:

    virtual ~CanSubtypeBase() {}

    typename ParentTypeContainer::iterator parentsBegin() const {
        return _container.begin();
    }

    typename ParentTypeContainer::iterator parentsEnd() const {
        return _container.end();
    }

    typename ParentTypeContainer::const_iterator cParentsBegin() const {
        return _container.cbegin();
    }

    typename ParentTypeContainer::const_iterator cParentEnd() const {
        return _container.cend();
    }

protected:

    CanSubtypeBase() {}

    ParentTypeContainer _container;
};

template<typename ParentType, template<typename, typename> class Container = std::vector>
class CanSubtype : public CanSubtypeBase<ParentType, Container> {
public:

    virtual ~CanSubtype() {}
};

template<typename ParentType>
class CanSubtype<ParentType, std::vector> : public CanSubtypeBase<ParentType, std::vector> {
    typedef CanSubtypeBase<ParentType, std::vector> Base;
public:

    virtual ~CanSubtype() {}

    void insertParent(ParentType* parent) {
        Base::_container.push_back(parent);
    }

    template<typename Iterator>
    void insertParents(const Iterator& begin, const Iterator& end) {
        Base::_container.insert(Base::_container.end(), begin, end);
    }

    void updateParents() {
        std::sort(Base::_container.begin(), Base::_container.end());
    }

    bool extends(ParentType* parent) const {
        return std::binary_search(Base::_container.cbegin(), Base::_container.cend(), parent);
    }
};

}

}

#endif
