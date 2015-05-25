//
//  InputSourceName.cpp
//  SFSL
//
//  Created by Romain Beguet on 09.04.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "InputSourceName.h"
#include "../Common/ManageableWrapper.h"

namespace sfsl {

namespace src {

InputSourceName::InputSourceName() : _name(&unknown) {

}

InputSourceName::InputSourceName(const std::string* name) : _name(name) {

}

InputSourceName InputSourceName::make(const CompCtx_Ptr& compilationContext, const std::string& name) {
    return InputSourceName(compilationContext.get()->memoryManager().New<common::ManageableWrapper<std::string>>(name)->getConstPtr());
}

const std::string& InputSourceName::getName() const {
    return *_name;
}

const std::string InputSourceName::unknown = "<unknown>";

}

}
