//
//  Kinds.cpp
//  SFSL
//
//  Created by Romain Beguet on 11.05.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "Kinds.h"

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

    virtual bool isSubKindOf(Kind*) const override {
        return false;
    }

    virtual std::string toString() const override {
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

ProperKind::ProperKind() {

}

ProperKind::~ProperKind() {

}

KIND_GENRE ProperKind::getKindGenre() const {
    return KIND_PROPER;
}

bool ProperKind::isSubKindOf(Kind* other) const {
    return other->getKindGenre() == KIND_PROPER;
}

std::string ProperKind::toString() const {
    return "*";
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

KIND_GENRE TypeConstructorKind::getKindGenre() const {
    return KIND_TYPE_CONSTRUCTOR;
}

bool TypeConstructorKind::isSubKindOf(Kind* other) const {
    if (TypeConstructorKind* tck = getIf<TypeConstructorKind>(other)) {

        const std::vector<Parameter>& others = tck->getArgKinds();

        if (others.size() != _args.size()) {
            return false;
        }

        for (size_t i = 0; i < _args.size(); ++i) {
            if (!_args[i].kind->isSubKindOf(others[i].kind) ||
                !isVarianceSubKind(_args[i].varianceType, others[i].varianceType)) {
                return false;
            }
        }

        return tck->getRetKind()->isSubKindOf(_ret);

    } else {
        return false;
    }
}

TypeConstructorKind::~TypeConstructorKind() {

}

std::string TypeConstructorKind::toString() const {
    std::string toRet = "[";
    for (size_t i = 0; i < _args.size(); ++i) {
        toRet += common::varianceTypeToString(_args[i].varianceType, true);
        toRet += _args[i].kind->toString();
        if (i != _args.size() - 1) {
            toRet += ", ";
        }
    }
    return toRet + "]->" + _ret->toString();
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
