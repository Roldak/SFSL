//
//  Bytecode.cpp
//  SFSL
//
//  Created by Romain Beguet on 22.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "Bytecode.h"
#include "../../../Utils/Utils.h"

#define ARG_SEP "\t\t"

namespace sfsl {

namespace bc {

// BYTE CODE INSTRUCTION

BCInstruction::~BCInstruction() {

}

std::string BCInstruction::toStringDetailed() const {
    std::ostringstream ss;
    appendTo(ss);
    return "<" + getSourceName().getName() + "@" + utils::T_toString(getStartPosition())
            + ":" + utils::T_toString(getEndPosition()) + ">\t" + ss.str();
}

// CREATE FUNCTION


MakeFunction::MakeFunction(size_t varCount, Label* end) : _varCount(varCount), _end(end) {

}

MakeFunction::~MakeFunction() {

}

void MakeFunction::appendTo(std::ostream& o) const {
    o << "mk_fn" << ARG_SEP << _varCount << ARG_SEP << _end->getName();
}

// STORE CONSTANT

StoreConst::StoreConst(size_t index) : _index(index) {

}

StoreConst::~StoreConst() {

}

void StoreConst::appendTo(std::ostream& o) const {
    o << "store_cst" << ARG_SEP << _index;
}

// LOAD CONSTANT

LoadConst::LoadConst(size_t index) : _index(index) {

}

LoadConst::~LoadConst() {

}

void LoadConst::appendTo(std::ostream &o) const {
    o << "load_cst" << ARG_SEP << _index;
}

// PUSH CONSTANT UNIT

PushConstUnit::PushConstUnit() {

}

PushConstUnit::~PushConstUnit() {

}

void PushConstUnit::appendTo(std::ostream &o) const {
    o << "push_unit";
}

// PUSH CONSTANT INTEGER

PushConstInt::PushConstInt(sfsl_int_t val) : _val(val) {

}

PushConstInt::~PushConstInt() {

}

void PushConstInt::appendTo(std::ostream& o) const {
    o << "push_i" << ARG_SEP << _val;
}

// PUSH CONSTANT INTEGER

PushConstReal::PushConstReal(sfsl_real_t val) : _val(val) {

}

PushConstReal::~PushConstReal() {

}

void PushConstReal::appendTo(std::ostream& o) const {
    o << "push_r" << ARG_SEP << _val;
}

// STACK LOAD

StackLoad::StackLoad(size_t index) : _index(index) {

}

StackLoad::~StackLoad() {

}

void StackLoad::appendTo(std::ostream& o) const {
    o << "load" << ARG_SEP << _index;
}

// POP

Pop::Pop() {

}

Pop::~Pop() {

}

void Pop::appendTo(std::ostream& o) const {
    o << "pop";
}

// RETURN

Return::Return() {

}

Return::~Return() {

}

void Return::appendTo(std::ostream& o) const {
    o << "ret";
}

// LABEL

Label::Label(const std::string& name) : _name(name) {

}

Label::~Label() {

}

const std::string& Label::getName() const {
    return _name;
}

void Label::appendTo(std::ostream &o) const {
    o << _name << ":";
}

// IF FALSE


IfFalse::IfFalse(Label *label) : _label(label) {

}

IfFalse::~IfFalse() {

}

void IfFalse::appendTo(std::ostream &o) const {
    o << "if_false" << ARG_SEP << _label->getName();
}

// JUMP

Jump::Jump(Label *label) : _label(label) {

}

Jump::~Jump() {

}

void Jump::appendTo(std::ostream &o) const {
    o << "jump" << ARG_SEP << _label->getName();
}

}

}
