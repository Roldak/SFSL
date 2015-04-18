//
//  TypeExpressions.h
//  SFSL
//
//  Created by Romain Beguet on 23.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__TypeExpressions__
#define __SFSL__TypeExpressions__

#include <iostream>
#include "Expressions.h"

namespace sfsl {

namespace ast {

/**
 * @brief The Class Declaration AST
 * Contains :
 *  - Its fields
 *  - Its definitions
 */
class ClassDecl : public Expression, public sym::Scoped {
public:

    ClassDecl(const std::string& name, const std::vector<TypeSpecifier*> fields, const std::vector<DefineDecl*> defs);
    virtual ~ClassDecl();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The list of fields declared in this class
     */
    const std::vector<TypeSpecifier*>& getFields() const;

    /**
     * @return The list of definitions declared in this class
     */
    const std::vector<DefineDecl*>& getDefs() const;

    /**
     * @return The name of the class
     */
    const std::string& getName() const;

private:

    std::string _name;
    std::vector<TypeSpecifier*> _fields;
    std::vector<DefineDecl*> _defs;
};

}

}

#endif
