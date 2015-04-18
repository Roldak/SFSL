//
//  TypeExpressions.cpp
//  SFSL
//
//  Created by Romain Beguet on 23.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "TypeExpressions.h"
#include "../Visitors/ASTVisitor.h"

namespace sfsl {

namespace ast {

// CLASS DECLARATION

ClassDecl::ClassDecl(const std::string& name,
                     const std::vector<TypeSpecifier *> fields,
                     const std::vector<DefineDecl *> defs)

    : _name(name), _fields(fields), _defs(defs)
{

}

ClassDecl::~ClassDecl() {

}

SFSL_AST_ON_VISIT_CPP(ClassDecl)

const std::vector<TypeSpecifier*>& ClassDecl::getFields() const {
    return _fields;
}

const std::vector<DefineDecl*>& ClassDecl::getDefs() const{
    return _defs;
}

const std::string &ClassDecl::getName() const {
    return _name;
}

}

}
