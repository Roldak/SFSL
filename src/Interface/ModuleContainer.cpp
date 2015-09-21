//
//  ModuleContainer.cpp
//  SFSL
//
//  Created by Romain Beguet on 21.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "ModuleContainer.h"

ModuleContainer::~ModuleContainer() {

}

sfsl::Module ModuleContainer::openModule(const std::string& name) {
    sfsl::Module m(createProxyModule(name));
    _modules.push_back(m);
    return m;
}

size_t ModuleContainer::openModulesCount() const {
    return _modules.size();
}

std::vector<sfsl::ast::ModuleDecl*> ModuleContainer::closeContainer(sfsl::common::AbstractMemoryManager& mngr) {
    std::vector<sfsl::ast::ModuleDecl*> modules;
    for (sfsl::Module module : _modules) {
        modules.push_back(buildModule(module));
    }
    return modules;
}
