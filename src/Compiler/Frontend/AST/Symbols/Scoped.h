//
//  Scoped.h
//  SFSL
//
//  Created by Romain Beguet on 13.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Scoped__
#define __SFSL__Scoped__

#include <iostream>

namespace sfsl {

namespace sym {

    class Scope;

    /**
     * @brief Interface that represents an object
     * to which can be attached a scope, e.g a sym::ModuleSymbol,
     * an ast::Block, etc.
     */
    class Scoped {
    public:

        virtual ~Scoped();

        /**
         * @param scope The scope to assign this object
         */
        void setScope(Scope* scope);

        /**
         * @return The scope assigned to this object
         */
        Scope* getScope() const;

    private:

        Scope* _scope;
    };

}

}

#endif
