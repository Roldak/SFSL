//
//  Annotation.cpp
//  SFSL
//
//  Created by Romain Beguet on 30.03.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#include "Annotations.h"

namespace sfsl {

namespace ast {

// ANNOTATION VALUE

Annotation::ArgumentValue::ArgumentValue() : tag(K_ANNOT_BOOL), value{.b = false} {

}

Annotation::ArgumentValue::ArgumentValue(bool b) : tag(K_ANNOT_BOOL), value{.b = b} {

}

Annotation::ArgumentValue::ArgumentValue(long i) : tag(K_ANNOT_INT), value{.i = i} {

}

Annotation::ArgumentValue::ArgumentValue(double r) : tag(K_ANNOT_REAL), value{.r = r} {

}

Annotation::ArgumentValue::ArgumentValue(const std::string& s) : tag(K_ANNOT_STRING), value{.s = s} {

}

// ANNOTATION

Annotation::Annotation(const std::string& name, const std::vector<Annotation::ArgumentValue>& args)
    : _name(name), _args(args), _used(false) {

}

Annotation::~Annotation() {

}

const std::string& Annotation::getName() const {
    return _name;
}

const std::vector<Annotation::ArgumentValue>& Annotation::getArgs() const {
    return _args;
}

void Annotation::setUsed() {
    _used = true;
}

bool Annotation::isUsed() const {
    return _used;
}

// ANNOTABLE

Annotable::Annotable() {

}

Annotable::~Annotable() {

}

void Annotable::setAnnotations(const std::vector<Annotation*>& annotations) {
    _annotations = annotations;
}

const std::vector<Annotation*>&Annotable::getAnnotations() const {
    return _annotations;
}

}

}
