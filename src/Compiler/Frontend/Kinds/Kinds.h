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
#include "../../Common/CompilationContext.h"
#include "../../Common/MemoryManageable.h"
#include "../Common/Miscellaneous.h"
#include "../Types/Environment.h"

namespace sfsl {

namespace kind {

enum KIND_GENRE {KIND_NYD, KIND_PROPER, KIND_TYPE_CONSTRUCTOR};

class Kind : public common::MemoryManageable {
public:
    virtual ~Kind();

    virtual KIND_GENRE getKindGenre() const = 0;
    virtual bool isSubKindOf(Kind* other, CompCtx_Ptr& ctx, bool checkBounds = false) const = 0;

    virtual Kind* substitute(const type::Environment& env, CompCtx_Ptr& ctx) = 0;
    virtual Kind* apply(CompCtx_Ptr& ctx) = 0;

    virtual std::string toString(bool withBoundsInformations = false, CompCtx_Ptr* shouldApply = nullptr) const = 0;


    static Kind* NotYetDefined();
};

class ProperKind : public Kind {
public:
    ProperKind(type::Type* lowerBound = nullptr, type::Type* upperBound = nullptr);

    virtual ~ProperKind();

    virtual KIND_GENRE getKindGenre() const override;
    virtual bool isSubKindOf(Kind* other, CompCtx_Ptr& ctx, bool checkBounds = false) const override;

    virtual ProperKind* substitute(const type::Environment& env, CompCtx_Ptr& ctx) override;
    virtual ProperKind* apply(CompCtx_Ptr& ctx) override;

    virtual std::string toString(bool withBoundsInformations = false, CompCtx_Ptr* shouldApply = nullptr) const override;

    type::Type* getLowerBound() const;
    type::Type* getUpperBound() const;

    static ProperKind* create();

private:

    type::Type* _lb;
    type::Type* _ub;
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
    virtual ~TypeConstructorKind();

    virtual KIND_GENRE getKindGenre() const override;
    virtual bool isSubKindOf(Kind* other, CompCtx_Ptr& ctx, bool checkBounds = false) const override;

    virtual TypeConstructorKind* substitute(const type::Environment& env, CompCtx_Ptr& ctx) override;
    virtual TypeConstructorKind* apply(CompCtx_Ptr& ctx) override;

    virtual std::string toString(bool withBoundsInformations = false, CompCtx_Ptr* shouldApply = nullptr) const override;

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
