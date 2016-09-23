//
//  HasCacheableCreatedType.h
//  SFSL
//
//  Created by Romain Beguet on 23.08.16.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__HasCacheableCreatedType__
#define __SFSL__HasCacheableCreatedType__

#include <iostream>

namespace sfsl {

namespace type {
class Type;
}

namespace ast {

/**
 * @brief An interface for ast nodes which can have
 * their type created by ASTTypeCreator cached
 */
class HasCacheableCreatedType {
public:
    HasCacheableCreatedType();
    virtual ~HasCacheableCreatedType();

    /**
     * @return True if a type has been cached already
     */
    bool hasCachedType() const;

    /**
     * @return The cached type (nullptr if the type has not been cached yet)
     */
    type::Type* getCachedType() const;

    /**
     * @brief Sets the cached type
     */
    void setCachedType(type::Type* tp);

private:

    type::Type* _cached;
};

class TypeExpression;

/**
 * @brief An interface for ast nodes which can hold
 * default types, i.e. KindSpecifyingExpressions, which
 * are created during NameAnalysis
 */
class CanHoldDefaultType {
public:
    CanHoldDefaultType();
    virtual ~CanHoldDefaultType();

    /**
     * @return True if the default type has already been created and is held by this object
     */
    bool holdsDefaultType() const;

    /**
     * @return The default type that this object holds
     */
    TypeExpression* getDefaultType() const;

    /**
     * @param texpr The default type to have this object hold
     */
    void setDefaultType(TypeExpression* texpr);

private:

    TypeExpression* _defaultType;
};

}

}

#endif
