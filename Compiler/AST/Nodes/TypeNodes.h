//
//  TypeNodes.h
//  SFSL
//
//  Created by Romain Beguet on 23.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__TypeNodes__
#define __SFSL__TypeNodes__

#include <iostream>
#include <vector>
#include "../../Common/CompilationContext.h"
#include "ASTNode.h"
#include "../../Types/Types.h"

namespace sfsl {

namespace ast {

    /**
     * @brief Interface that represents a node that has a type,
     * like a binary expression, an identifier, etc.
     */
    class Typed {
    public:
        virtual ~Typed();

        /**
         * @param type The type to set
         */
        void setType(type::Type* type);

        /**
         * @return The type of the object
         */
        virtual type::Type* type();

    protected:

        type::Type* _type;
    };
}

}

#endif
