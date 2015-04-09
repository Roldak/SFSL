//
//  SFSLSourceName.cpp
//  SFSL
//
//  Created by Romain Beguet on 09.04.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "SFSLSourceName.h"
#include "../Common/ManageableWrapper.h"

namespace sfsl {

namespace src {

SFSLSourceName::SFSLSourceName() : _name(&unknown) {

}

SFSLSourceName::SFSLSourceName(const std::string* name) : _name(name) {

}

SFSLSourceName SFSLSourceName::make(const std::shared_ptr<common::CompilationContext>& compilationContext, const std::string& name) {
    return SFSLSourceName(compilationContext.get()->memoryManager().New<common::ManageableWrapper<std::string>>(name)->getConstPtr());
}

const std::string& SFSLSourceName::getName() const {
    return *_name;
}

const std::string SFSLSourceName::unknown = "<unknown>";

}

}
