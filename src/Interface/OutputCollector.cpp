//
//  OutputCollector.cpp
//  SFSL
//
//  Created by Romain Beguet on 16.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "api/AbstractOutputCollector.h"
#include "api/StandartOutputCollector.h"
#include "api/PhaseContext.h"

#include "Compiler/Backend/BytecodeGenerator.h"

namespace sfsl {

// ABSTRACT OUTPUT COLLECTOR

AbstractOutputCollector::AbstractOutputCollector() {

}

AbstractOutputCollector::~AbstractOutputCollector() {

}

void AbstractOutputCollector::collect(PhaseContext& pctx) {

}

// BYTECODE COLLECTOR

ByteCodeCollector::ByteCodeCollector() {

}

ByteCodeCollector::~ByteCodeCollector() {

}

void ByteCodeCollector::collect(PhaseContext& pctx) {
    _result.clear();

    out::LinkedListOutput<bc::BCInstruction*>* out = pctx.require<out::LinkedListOutput<bc::BCInstruction*>>("out");
    std::vector<bc::BCInstruction*> instrs(out->toVector());

    for (bc::BCInstruction* instr : instrs) {
        _result.push_back(instr->toStringDetailed());
    }
}

std::vector<std::string> ByteCodeCollector::get() const {
    return _result;
}

// ERROR COUNT COLLECTOR

ErrorCountCollector::ErrorCountCollector() : _errCount(0) {

}

ErrorCountCollector::~ErrorCountCollector() {

}

void ErrorCountCollector::collect(PhaseContext& pctx) {
    CompCtx_Ptr ctx = *pctx.require<CompCtx_Ptr>("ctx");
    _errCount = ctx->reporter().getErrorCount();
}

size_t ErrorCountCollector::get() const {
    return _errCount;
}

}
