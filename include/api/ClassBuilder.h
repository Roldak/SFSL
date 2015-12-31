//
//  ClassBuilder.h
//  SFSL
//
//  Created by Romain Beguet on 30.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_ClassBuilder__
#define __SFSL__API_ClassBuilder__

#include <iostream>
#include "SetVisibilities.h"

#include "Type.h"

DECL_PRIVATE_IMPL_FOR(ClassBuilder)

namespace sfsl {

class SFSL_API_PUBLIC ClassBuilder final {
public:

    ~ClassBuilder();

    ClassBuilder& setAbstract(bool value);
    ClassBuilder& addField(const std::string& fieldName, Type fieldType);
    ClassBuilder& addExternDef(const std::string& defName, Type defType, bool isRedef);
    ClassBuilder& addAbstractDef(const std::string& defName, Type defType);

    Type build() const;

private:
    friend class Compiler;

    ClassBuilder(PRIVATE_IMPL_PTR(ClassBuilder) impl);

    PRIVATE_IMPL_PTR(ClassBuilder) _impl;
};

}

#endif
