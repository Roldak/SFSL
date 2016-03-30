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


Annotation::ArgumentValue::Value::Value() {

}

Annotation::ArgumentValue::Value::~Value() {

}

Annotation::ArgumentValue::ArgumentValue() : tag(K_ANNOT_BOOL) {
    value.b = false;
}

Annotation::ArgumentValue::ArgumentValue(sfsl_bool_t b) : tag(K_ANNOT_BOOL) {
    value.b = b;
}

Annotation::ArgumentValue::ArgumentValue(sfsl_int_t i) : tag(K_ANNOT_INT) {
    value.i = i;
}

Annotation::ArgumentValue::ArgumentValue(sfsl_real_t r) : tag(K_ANNOT_REAL) {
    value.r = r;
}

Annotation::ArgumentValue::ArgumentValue(const std::string& s) : tag(K_ANNOT_STRING) {
    new (&value.s) std::string(s);
}

Annotation::ArgumentValue::ArgumentValue(const Annotation::ArgumentValue& other) : tag(other.tag) {
    switch (tag) {
    case K_ANNOT_BOOL:
        value.b = other.value.b;
        break;
    case K_ANNOT_INT:
        value.i = other.value.i;
        break;
    case K_ANNOT_REAL:
        value.r = other.value.r;
        break;
    case K_ANNOT_STRING:
        new (&value.s) std::string(other.value.s);
        break;
    }
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
