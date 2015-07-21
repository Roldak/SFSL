//
//  CodeGenOutput.h
//  SFSL
//
//  Created by Romain Beguet on 21.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__CodeGenOutput__
#define __SFSL__CodeGenOutput__

#include <iostream>

namespace sfsl {

namespace out {

    /**
     * @brief Abstract class representing the destination of the generated code
     */
    class CodeGenOutput {
    public:

        CodeGenOutput();

        virtual ~CodeGenOutput();

    protected:

    };
}

}

#endif
