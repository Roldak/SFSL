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
#include <vector>
#include "Utils/Utils.h"

namespace sfsl {

namespace out {

    template<typename T>
    /**
     * @brief Interface representing the destination of the generated code
     */
    class CodeGenOutput {
    public:

        virtual ~CodeGenOutput();

        /**
         * @brief Appends a value to the destination object
         * @param t The value to be added
         * @return This
         */
        virtual CodeGenOutput& operator <<(const T& t) = 0;

        /**
         * @return A visualization of the generated bytecode through a string
         */
        virtual std::string toString() = 0;
    };

    template<typename T>
    /**
     * @brief Implementation of the CodeGenOutput interface writing the emitted code into a vector
     */
    class VectorOutput : CodeGenOutput<T> {
    public:

        VectorOutput() {}

        virtual ~VectorOutput() {}

        virtual CodeGenOutput& operator <<(const T& t) override {
            _vec.push_back(t);
        }

        virtual std::string toString() override {
            std::string toRet;
            for (const T& t : _vec) {
                toRet += utils::T_toString(t);
            }
            return toRet;
        }

    protected:

        std::vector<T> _vec;
    };
}

}

#endif
