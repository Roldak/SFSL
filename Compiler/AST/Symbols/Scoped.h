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

    class Scoped {
    public:

        virtual ~Scoped();

        void setScope(Scope* scope);
        Scope* getScope() const;

    private:

        Scope* _scope;
    };

}

}

#endif
