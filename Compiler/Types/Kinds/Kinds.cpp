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

TypeKind::TypeKind() {

}

TypeKind::~TypeKind() {

}

KIND_GENRE TypeKind::getKindGenre() const {
    return TYPE_KIND;
}

bool TypeKind::isSubKindOf(Kind* other) const {
    return true;
}

std::string TypeKind::toString() const {
    return "*";
}

// no need to allocate a TypeKind everytime
TypeKind* TypeKind::create() {
    static kind::TypeKind k;
    return &k;
}

// TYPE CONSTRUCTOR KIND

TypeConstructorKind::TypeConstructorKind(const std::vector<Kind*>& args, Kind* ret) : _args(args), _ret(ret) {

}

KIND_GENRE TypeConstructorKind::getKindGenre() const {
    return TYPE_CONSTRUCTOR_KIND;
}

bool TypeConstructorKind::isSubKindOf(Kind* other) const {
    if (TypeConstructorKind* tck = getIf<TypeConstructorKind>(other)) {

        const std::vector<Kind*>& others = tck->getArgKinds();

        if (others.size() != _args.size()) {
            return false;
        }

        for (size_t i = 0; i < _args.size(); ++i) {
            if (!_args[i]->isSubKindOf(others[i])) {
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
        toRet += _args[i]->toString();
        if (i != _args.size() - 1) {
            toRet += ", ";
        }
    }
    return toRet + "]->" + _ret->toString();
}

const std::vector<Kind*>& TypeConstructorKind::getArgKinds() const {
    return _args;
}

Kind* TypeConstructorKind::getRetKind() const {
    return _ret;
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
