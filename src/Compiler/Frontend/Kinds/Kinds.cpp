//
//  Kinds.cpp
//  SFSL
//
//  Created by Romain Beguet on 11.05.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "Kinds.h"
#include "../Types/Types.h"

namespace sfsl {

namespace kind {

// KIND NOT YET DEFINED

class KindNotYetDefined : public Kind {
public:

    KindNotYetDefined() {}

    virtual ~KindNotYetDefined() {}

    virtual KIND_GENRE getKindGenre() const override {
        return KIND_NYD;
    }

    virtual bool isSubKindOf(Kind*, bool) const override {
        return false;
    }

    virtual KindNotYetDefined* substitute(const type::SubstitutionTable&, CompCtx_Ptr&) override {
        return this;
    }

    virtual KindNotYetDefined* apply(CompCtx_Ptr&) override {
        return this;
    }

    virtual std::string toString(bool) const override {
        return "[not yet defined]";
    }
};

// KIND

Kind::~Kind() {

}

Kind* Kind::NotYetDefined() {
    static KindNotYetDefined nyd;
    return &nyd; // all we want is a unique memory area
}

// TYPE KIND

ProperKind::ProperKind(type::Type* lowerBound, type::Type* upperBound)
    : _lb(lowerBound), _ub(upperBound) {

}

ProperKind::~ProperKind() {

}

KIND_GENRE ProperKind::getKindGenre() const {
    return KIND_PROPER;
}

bool ProperKind::isSubKindOf(Kind* other, bool checkBounds) const {
    if (!checkBounds) {
        return other->getKindGenre() == kind::KIND_PROPER;
    } else if (kind::ProperKind* opk = kind::getIf<kind::ProperKind>(other)) {
        if (!opk->getLowerBound() && !opk->getUpperBound()) { // since this scenario is pretty common, special case an early return for it
            return true;
        }

        type::Type* otherLb = opk->getLowerBound();
        type::Type* otherUb = opk->getUpperBound();

        if (_lb && otherLb) {
            if (!otherLb->isSubTypeOf(_lb)) {
                return false;
            }
        } else if (otherLb) {
            return false;
        }

        if (_ub && otherUb) {
            if (!_ub->isSubTypeOf(otherUb)) {
                return false;
            }
        } else if (otherUb) {
            return false;
        }

        return true;
    }
    return false;
}

ProperKind* ProperKind::substitute(const type::SubstitutionTable& table, CompCtx_Ptr& ctx) {
    if (!_lb && !_ub) {
        return this;
    } else {
        type::Type* newLb = _lb ? type::Type::findSubstitution(table, _lb)->substitute(table, ctx) : nullptr;
        type::Type* newUb = _ub ? type::Type::findSubstitution(table, _ub)->substitute(table, ctx) : nullptr;
        return ctx->memoryManager().New<ProperKind>(newLb, newUb);
    }
}

ProperKind* ProperKind::apply(CompCtx_Ptr& ctx) {
    if (!_lb && !_ub) {
        return this;
    } else {
        type::Type* newLb = _lb ? _lb->apply(ctx) : nullptr;
        type::Type* newUb = _ub ? _ub->apply(ctx) : nullptr;
        return ctx->memoryManager().New<ProperKind>(newLb, newUb);
    }
}

const std::string lessThan = " < ";

std::string ProperKind::toString(bool withBoundsInformations) const {
    std::string lbStr = _lb ? _lb->toString() + lessThan : "";
    std::string ubStr = _ub ? lessThan + _ub->toString() : "";
    if (withBoundsInformations) {
        return lbStr + "*" + ubStr;
    } else {
        return "*";
    }
}

type::Type* ProperKind::getLowerBound() const {
    return _lb;
}

type::Type* ProperKind::getUpperBound() const {
    return _ub;
}

// no need to allocate a TypeKind everytime
ProperKind* ProperKind::create() {
    static kind::ProperKind k;
    return &k;
}

// TYPE CONSTRUCTOR KIND

TypeConstructorKind::Parameter::Parameter() : varianceType(common::VAR_T_NONE), kind(nullptr) {

}

TypeConstructorKind::Parameter::Parameter(common::VARIANCE_TYPE vt, Kind* kind) : varianceType(vt), kind(kind) {

}

TypeConstructorKind::TypeConstructorKind(const std::vector<Parameter>& args, Kind* ret) : _args(args), _ret(ret) {

}

TypeConstructorKind::~TypeConstructorKind() {

}

KIND_GENRE TypeConstructorKind::getKindGenre() const {
    return KIND_TYPE_CONSTRUCTOR;
}

bool TypeConstructorKind::isSubKindOf(Kind* other, bool checkBounds) const {
    if (TypeConstructorKind* tck = getIf<TypeConstructorKind>(other)) {
        const std::vector<Parameter>& others = tck->getArgKinds();

        if (others.size() != _args.size()) {
            return false;
        }

        for (size_t i = 0; i < _args.size(); ++i) {
            if (!_args[i].kind->isSubKindOf(others[i].kind, checkBounds) ||
                !isVarianceSubKind(_args[i].varianceType, others[i].varianceType)) {
                return false;
            }
        }

        return _ret->isSubKindOf(tck->getRetKind(), checkBounds);
    }
    return false;
}

TypeConstructorKind* TypeConstructorKind::substitute(const type::SubstitutionTable& table, CompCtx_Ptr& ctx) {
    std::vector<Parameter> newParams(_args.size());
    for (size_t i = 0; i < _args.size(); ++i) {
        newParams[i].varianceType = _args[i].varianceType;
        newParams[i].kind = _args[i].kind->substitute(table, ctx);
    }
    return ctx->memoryManager().New<TypeConstructorKind>(newParams, _ret->substitute(table, ctx));
}

TypeConstructorKind* TypeConstructorKind::apply(CompCtx_Ptr& ctx) {
    std::vector<Parameter> newParams(_args.size());
    for (size_t i = 0; i < _args.size(); ++i) {
        newParams[i].varianceType = _args[i].varianceType;
        newParams[i].kind = _args[i].kind->apply(ctx);
    }
    return ctx->memoryManager().New<TypeConstructorKind>(newParams, _ret->apply(ctx));
}

std::string TypeConstructorKind::toString(bool withBoundsInformations) const {
    std::string toRet = "[";
    for (size_t i = 0; i < _args.size(); ++i) {
        toRet += common::varianceTypeToString(_args[i].varianceType, true);
        toRet += _args[i].kind->toString(withBoundsInformations);
        if (i != _args.size() - 1) {
            toRet += ", ";
        }
    }
    return toRet + "]->" + _ret->toString(withBoundsInformations);
}

const std::vector<TypeConstructorKind::Parameter>& TypeConstructorKind::getArgKinds() const {
    return _args;
}

Kind* TypeConstructorKind::getRetKind() const {
    return _ret;
}

bool TypeConstructorKind::isVarianceSubKind(common::VARIANCE_TYPE a, common::VARIANCE_TYPE b) {
    if (a == b) {
        return true;
    } else if (b == common::VAR_T_NONE) {
        return true;
    } else {
        return false;
    }
}

// KINDED

Kinded::Kinded() : _kind(Kind::NotYetDefined()) {

}

Kinded::~Kinded() {

}

void Kinded::setKind(Kind* kind) {
    _kind = kind;
}

Kind* Kinded::kind() const {
    return _kind;
}

}

}
