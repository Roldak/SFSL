//
//  TypeParametrizable.h
//  SFSL
//
//  Created by Romain Beguet on 06.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__TypeParametrizable__
#define __SFSL__TypeParametrizable__

#include <vector>

namespace sfsl {

namespace sym {
class TypeSymbol;
}

namespace type {

class TypeParametrizable {
public:
    TypeParametrizable();
    virtual ~TypeParametrizable();

    void setDependsOn(sym::TypeSymbol* type);

    const std::vector<sym::TypeSymbol*>& getDependencies() const;

private:

    std::vector<sym::TypeSymbol*> _types;
};

}

}

#endif
