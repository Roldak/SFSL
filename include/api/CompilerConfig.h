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
#include <memory>
#include "SetVisibilities.h"
#include "AbstractReporter.h"
#include "AbstractPrimitiveNamer.h"

namespace sfsl {

class SFSL_API_PUBLIC CompilerConfig final {
public:
    CompilerConfig(AbstractReporter* rep = nullptr, common::AbstractPrimitiveNamer* namer = nullptr, size_t chunkSize = 2048);
    ~CompilerConfig();

    void setChunkSize(size_t chunkSize);
    size_t getChunkSize() const;

    void setReporter(AbstractReporter* rep);
    AbstractReporter* getReporter() const;

    void setPrimitiveNamer(common::AbstractPrimitiveNamer* namer);
    common::AbstractPrimitiveNamer* getPrimitiveNamer() const;

private:

    AbstractReporter* _rep;
    common::AbstractPrimitiveNamer* _namer;
    size_t _chunkSize;
};

}

#endif
