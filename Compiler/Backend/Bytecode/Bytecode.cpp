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


MakeFunction::MakeFunction(size_t varCount) : _varCount(varCount) {

}

MakeFunction::~MakeFunction() {

}

void MakeFunction::appendTo(std::ostream& o) const {
    o << "mk_fn" << ARG_SEP << _varCount;
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

}

}
