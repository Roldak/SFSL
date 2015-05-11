//
//  Kinds.h
//  SFSL
//
//  Created by Romain Beguet on 11.05.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Kinds__
#define __SFSL__Kinds__

#include <iostream>
#include "../../Common/MemoryManageable.h"

namespace sfsl {

namespace kind {

class Kind;

class Kind : public common::MemoryManageable {
public:
    virtual ~Kind();

    virtual std::string toString() const = 0;

    static Kind* NotYetDefined();
};

class TypeKind : public Kind {
public:
    TypeKind();

    virtual ~TypeKind();

    virtual std::string toString() const override;
};

class TypeConstructorKind : public Kind {
public:
    TypeConstructorKind();

    virtual ~TypeConstructorKind();

    virtual std::string toString() const override;
};

/**
 * @brief Interface that represents a node that has a kind,
 * like a type expression, or the type symbol.
 */
class Kinded {
public:
    Kinded();

    virtual ~Kinded();

    /**
     * @param kind The kind to set
     */
    void setKind(Kind* kind);

    /**
     * @return The kind of the object
     */
    Kind* kind() const;

protected:

    Kind* _kind;
};

}

}

#endif
