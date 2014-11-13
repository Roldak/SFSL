//
//  Token.h
//  SFSL
//
//  Created by Romain Beguet on 12.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Token__
#define __SFSL__Token__

#include "../../Common/MemoryManageable.h"
#include "../../Common/Positionnable.h"

namespace sfsl {

namespace tok {

    /**
     * @brief Represents an abstract Token
     */
    class Token : public common::MemoryManageable, public common::Positionnable<Token> {
    public:

    private:

    };


}

}

#endif
