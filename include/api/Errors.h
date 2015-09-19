//
//  Errors.h
//  SFSL
//
//  Created by Romain Beguet on 19.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_Errors__
#define __SFSL__API_Errors__

#include <stdexcept>
#include "set_visibilities.h"

namespace sfsl {

class SFSL_API_PUBLIC CompileError final : public std::runtime_error {
public:
    explicit CompileError(const std::string& msg);
    ~CompileError();
};

}

#endif
