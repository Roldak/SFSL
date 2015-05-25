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
#include <vector>
#include "../Common/MemoryManageable.h"

namespace sfsl {

namespace kind {

enum KIND_GENRE {KIND_NYD, TYPE_KIND, TYPE_CONSTRUCTOR_KIND};

class Kind;

class Kind : public common::MemoryManageable {
public:
    virtual ~Kind();

    virtual KIND_GENRE getKindGenre() const = 0;
    virtual bool isSubKindOf(Kind* other) const = 0;
    virtual std::string toString() const = 0;

    static Kind* NotYetDefined();
};

class TypeKind : public Kind {
public:
    TypeKind();

    virtual ~TypeKind();

    virtual KIND_GENRE getKindGenre() const override;
    virtual bool isSubKindOf(Kind* other) const override;
    virtual std::string toString() const override;

    static TypeKind* create();
};

class TypeConstructorKind : public Kind {
public:
    TypeConstructorKind(const std::vector<Kind*>& args, Kind* ret);

    virtual KIND_GENRE getKindGenre() const override;
    virtual bool isSubKindOf(Kind* other) const override;
    virtual ~TypeConstructorKind();

    virtual std::string toString() const override;

    const std::vector<Kind*>& getArgKinds() const;
    Kind* getRetKind() const;

private:

    std::vector<Kind*> _args;
    Kind* _ret;
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

template<typename T>
inline T* getIf(const Kind* k) {
    return nullptr;
}

template<>
inline TypeKind* getIf(const Kind* k) {
    return k->getKindGenre() == TYPE_KIND ? (TypeKind*)k : nullptr;
}

template<>
inline TypeConstructorKind* getIf(const Kind* k) {
    return k->getKindGenre() == TYPE_CONSTRUCTOR_KIND ? (TypeConstructorKind*)k : nullptr;
}

}

}

#endif
