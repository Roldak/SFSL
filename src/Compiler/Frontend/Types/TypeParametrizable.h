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
#include "../Common/Miscellaneous.h"

namespace sfsl {

namespace sym {
class TypeSymbol;
}

namespace type {

/**
 * @brief Interface which represents an object that can depend on type parameters.
 * Concrete implementations are: ClassDecl, TypeConstructorCreation and FunctionCreation.
 */
class TypeParametrizable {
public:
    struct Parameter {
        Parameter();
        Parameter(common::VARIANCE_TYPE varianceType, sym::TypeSymbol* symbol);

        common::VARIANCE_TYPE varianceType;
        sym::TypeSymbol* symbol;
    };

    virtual ~TypeParametrizable();

    /**
     * @param types The type parameters of this object
     */
    void setParameters(const std::vector<Parameter>& types);

    /**
     * @return All the type parameters that this object depends on
     */
    const std::vector<Parameter>& getParameters() const;

private:

    std::vector<Parameter> _types;
};

}

}

#endif
