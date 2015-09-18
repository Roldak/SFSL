//
//  CompilerConfig.h
//  SFSL
//
//  Created by Romain Beguet on 18.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "api/CompilerConfig.h"
#include "api/set_visibilities.h"

namespace sfsl {

CompilerConfig::CompilerConfig(size_t chunkSize) : _chunkSize(chunkSize) {

}

CompilerConfig::~CompilerConfig() {

}

void CompilerConfig::setChunkSize(size_t chunkSize) {
    _chunkSize = chunkSize;
}

size_t CompilerConfig::getChunkSize() const {
    return _chunkSize;
}

}
