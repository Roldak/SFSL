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
/**
 * @brief Base trait of the CanSubtype trait
 * @see CanSubtype
 */
class CanSubtypeBase {
    typedef Container<ParentType*, std::allocator<ParentType*>> ParentTypeContainer;

public:

    virtual ~CanSubtypeBase() {}

    /**
     * @return An iterator to the first parent of this object
     */
    typename ParentTypeContainer::iterator parentBegin() const {
        return _container.begin();
    }

    /**
     * @return An iterator to the last + 1 parent of this object
     */
    typename ParentTypeContainer::iterator parentEnd() const {
        return _container.end();
    }

    /**
     * @return A const iterator to the first parent of this object
     */
    typename ParentTypeContainer::const_iterator cParentBegin() const {
        return _container.cbegin();
    }

    /**
     * @return A const iterator to the last + 1 parent of this object
     */
    typename ParentTypeContainer::const_iterator cParentEnd() const {
        return _container.cend();
    }

protected:

    CanSubtypeBase() {}

    ParentTypeContainer _container;
};

template<typename ParentType, template<typename, typename> class Container = std::vector>
/**
 * @brief Trait which can be extended by classes to declare they can subtype the type ParentType.
 * This trait can be specialized to implement your own container type. The default specialization uses
 * a sorted std::vector.
 */
class CanSubtype : public CanSubtypeBase<ParentType, Container> {
public:

    virtual ~CanSubtype() {}
};

template<typename ParentType>
/**
 * @brief Default specialization of the CanSubtype trait, using as the underlying container
 * a std::vector and the std::binary_search function once the vector is sorted.
 */
class CanSubtype<ParentType, std::vector> : public CanSubtypeBase<ParentType, std::vector> {
    typedef CanSubtypeBase<ParentType, std::vector> Base;
public:

    virtual ~CanSubtype() {}

    /**
     * @brief Add a new parent to this object
     * @param parent The parent to add
     */
    void insertParent(ParentType* parent) {
        Base::_container.push_back(parent);
    }

    template<typename Iterator>
    /**
     * @brief Insert multiple parents to this object
     * @param begin The begin iterator of the collection to add
     * @param end The end iterator of the collection to add
     */
    void insertParents(const Iterator& begin, const Iterator& end) {
        Base::_container.insert(Base::_container.end(), begin, end);
    }

    /**
     * @brief Sorts the underlying vector, so that binary search can be used
     * to determine sub type validity
     */
    void updateParents() {
        std::sort(Base::_container.begin(), Base::_container.end());
    }

    /**
     * @brief Can be used to determine if the given ParentType instance is among the parent of this object
     *
     * <b>Note :</b> Has log(N) complexity, where N is the number of parent. It uses std::binary_search to achieve
     * this complexity, it is therefore important to have called updateParents to sort the vector before using extends.
     *
     * @param parent The instance to test as parent of this object
     * @return True if the given ParentType instance is a parent of this object
     */
    bool extends(ParentType* parent) const {
        return std::binary_search(Base::_container.cbegin(), Base::_container.cend(), parent);
    }
};

}

}

#endif
