//
//  CompilerConfig.h
//  SFSL
//
//  Created by Romain Beguet on 18.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "api/CompilerConfig.h"

namespace sfsl {

CompilerConfig::CompilerConfig(AbstractReporter* rep, common::PrimitiveNamer* namer, size_t chunkSize)
    : _rep(rep), _namer(namer), _chunkSize(chunkSize) {

}

CompilerConfig::~CompilerConfig() {

}

void CompilerConfig::setChunkSize(size_t chunkSize) {
    _chunkSize = chunkSize;
}

size_t CompilerConfig::getChunkSize() const {
    return _chunkSize;
}

void CompilerConfig::setReporter(AbstractReporter* rep) {
    _rep = rep;
}

AbstractReporter* CompilerConfig::getReporter() const {
    return _rep;
}

void CompilerConfig::setPrimitiveNamer(common::PrimitiveNamer* namer) {
    _namer = namer;
}

common::PrimitiveNamer* CompilerConfig::getPrimitiveNamer() const {
    return _namer;
}

}
