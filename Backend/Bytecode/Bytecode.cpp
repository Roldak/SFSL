//
//  Bytecode.cpp
//  SFSL
//
//  Created by Romain Beguet on 22.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "Bytecode.h"
#include "../../Utils/Utils.h"

#define ARG_SEP "\t"

namespace sfsl {

namespace bc {

// BYTE CODE INSTRUCTION

BCInstruction::~BCInstruction() {

}

std::string BCInstruction::toStringDetailed() const {
    std::ostringstream ss;
    toString(ss);
    return "<" + getSourceName().getName() + "@" + utils::T_toString(getStartPosition())
            + ":" + utils::T_toString(getEndPosition()) + ">\t" + ss.str();
}

// BYTE CODE PUSH CONSTANT INTEGER

PushConstInt::PushConstInt(sfsl_int_t val) : _val(val) {

}

PushConstInt::~PushConstInt() {

}

void PushConstInt::toString(std::ostream &o) const {
    o << "push_i" << ARG_SEP << utils::T_toString(_val);
}

// BYTE CODE PUSH CONSTANT INTEGER

PushConstReal::PushConstReal(sfsl_real_t val) : _val(val) {

}

PushConstReal::~PushConstReal() {

}

void PushConstReal::toString(std::ostream &o) const {
    o << "push_r" << ARG_SEP << utils::T_toString(_val);
}

}

}
