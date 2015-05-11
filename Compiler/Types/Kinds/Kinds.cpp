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

    virtual std::string toString() const {
        return "<not yet defined>";
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

std::string TypeKind::toString() const {
    return "#";
}

// TYPE CONSTRUCTOR KIND

TypeConstructorKind::TypeConstructorKind() {

}

TypeConstructorKind::~TypeConstructorKind() {

}

std::string TypeConstructorKind::toString() const {
    return "[#]->#";
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
