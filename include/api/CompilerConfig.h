//
//  CompilerConfig.h
//  SFSL
//
//  Created by Romain Beguet on 18.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_CompilerConfig__
#define __SFSL__API_CompilerConfig__

#include <iostream>
#include "set_visibilities.h"

namespace sfsl {

class SFSL_API_PUBLIC CompilerConfig final {
public:
    CompilerConfig(size_t chunkSize = 2048);
    ~CompilerConfig();

    void setChunkSize(size_t chunkSize);
    size_t getChunkSize() const;

private:

    size_t _chunkSize;
};

}

#endif
