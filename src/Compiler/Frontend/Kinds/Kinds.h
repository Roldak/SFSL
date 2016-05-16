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
#include "../../Common/MemoryManageable.h"
#include "../Common/Miscellaneous.h"

namespace sfsl {

namespace type {
class ProperType;
}

namespace kind {

enum KIND_GENRE {KIND_NYD, KIND_PROPER, KIND_TYPE_CONSTRUCTOR};

class Kind : public common::MemoryManageable {
public:
    virtual ~Kind();

    virtual KIND_GENRE getKindGenre() const = 0;
    virtual bool isSubKindOf(Kind* other) const = 0;
    virtual std::string toString() const = 0;

    static Kind* NotYetDefined();
};

class ProperKind : public Kind {
public:
    ProperKind(type::ProperType* lowerBound = nullptr, type::ProperType* upperBound = nullptr);

    virtual ~ProperKind();

    virtual KIND_GENRE getKindGenre() const override;
    virtual bool isSubKindOf(Kind* other) const override;
    virtual std::string toString() const override;

    type::ProperType* getLowerBound() const;
    type::ProperType* getUpperBound() const;

    static ProperKind* create();

private:

    type::ProperType* _lb;
    type::ProperType* _ub;
};

class TypeConstructorKind : public Kind {
public:
    struct Parameter {
        Parameter();
        Parameter(common::VARIANCE_TYPE vt, Kind* kind);

        common::VARIANCE_TYPE varianceType;
        Kind* kind;
    };

    TypeConstructorKind(const std::vector<Parameter>& args, Kind* ret);

    virtual KIND_GENRE getKindGenre() const override;
    virtual bool isSubKindOf(Kind* other) const override;
    virtual ~TypeConstructorKind();

    virtual std::string toString() const override;

    const std::vector<Parameter>& getArgKinds() const;
    Kind* getRetKind() const;

private:

    static bool isVarianceSubKind(common::VARIANCE_TYPE a, common::VARIANCE_TYPE b);

    std::vector<Parameter> _args;
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
inline ProperKind* getIf(const Kind* k) {
    return k->getKindGenre() == KIND_PROPER ? (ProperKind*)k : nullptr;
}

template<>
inline TypeConstructorKind* getIf(const Kind* k) {
    return k->getKindGenre() == KIND_TYPE_CONSTRUCTOR ? (TypeConstructorKind*)k : nullptr;
}

}

}

#endif
