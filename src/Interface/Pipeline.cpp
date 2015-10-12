//
//  Pipeline.cpp
//  SFSL
//
//  Created by Romain Beguet on 04.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "api/Pipeline.h"

namespace sfsl {

Pipeline::Pipeline() {

}

Pipeline::~Pipeline() {

}

Pipeline& Pipeline::insert(std::shared_ptr<Phase> phase) {
    _phases.insert(phase);
    return *this;
}

}
