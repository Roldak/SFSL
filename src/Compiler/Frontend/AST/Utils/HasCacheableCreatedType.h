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

}

}

#endif
