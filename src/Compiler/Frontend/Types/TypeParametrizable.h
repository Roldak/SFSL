//
//  TypeParametrizable.h
//  SFSL
//
//  Created by Romain Beguet on 06.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__TypeParametrizable__
#define __SFSL__TypeParametrizable__

#include <set>

namespace sfsl {

namespace sym {
class TypeSymbol;
}

namespace type {

/**
 * @brief Interface which represents an object that can depend on type symbols.
 * Concrete implementations are: ClassDecl, TraitDecl, TypeConstructorCreation and FunctionCreation.
 */
class TypeParametrizable {
public:
    TypeParametrizable();
    virtual ~TypeParametrizable();

    /**
     * @param type The additionnal dependency of this object
     */
    void setDependsOn(sym::TypeSymbol* type);

    /**
     * @return All the types that this object depends on
     */
    const std::set<sym::TypeSymbol*>& getDependencies() const;

private:

    std::set<sym::TypeSymbol*> _types;
};

}

}

#endif
