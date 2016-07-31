//
//  ModuleContainer.h
//  SFSL
//
//  Created by Romain Beguet on 21.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_ModuleContainer__
#define __SFSL__API_ModuleContainer__

#include "api/Module.h"
#include "Compiler/Frontend/AST/Nodes/Program.h"

class ModuleContainer {
public:
    virtual ~ModuleContainer();

    sfsl::Module openModule(const std::string& name);
    size_t openModulesCount() const;

    std::vector<sfsl::ast::ModuleDecl*> closeContainer(sfsl::common::AbstractMemoryManager& mngr);

    virtual sfsl::Module createProxyModule(const std::string& name) const = 0;
    virtual sfsl::ast::ModuleDecl* buildModule(sfsl::Module m) = 0;

private:

    std::vector<sfsl::Module> _modules;
};

#endif
