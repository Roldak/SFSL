//
//  InputSourceName.h
//  SFSL
//
//  Created by Romain Beguet on 09.04.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__InputSourceName__
#define __SFSL__InputSourceName__

#include <iostream>
#include "../../Common/CompilationContext.h"

namespace sfsl {

namespace src {

/**
 * @brief Simple class containing the name of an InputSource, which can be automatically managed
 */
class InputSourceName final {
public:

    InputSourceName();

    /**
     * @brief Creates a new InputSourceName object and make it managed automatically
     * @param compilationContext The Compilation context from which to instantiate the object
     * @param name The name to the source file
     * @return The newly created InputSourceName
     */
    static InputSourceName make(const CompCtx_Ptr& compilationContext, const std::string& name);

    const std::string& getName() const;

private:

    InputSourceName(const std::string* name);

    static const std::string unknown;

    const std::string* _name;
};

}

}

#endif
