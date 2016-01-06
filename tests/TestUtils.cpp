//
//  TestUtils.h
//  SFSL
//
//  Created by Romain Beguet on 01.01.16.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "TestUtils.h"

namespace sfsl {

namespace test {

void createFunctionClass(Compiler cmp, Module dest, size_t nbArg) {
    std::string name = "Func";
    name += std::to_string(nbArg);

    std::vector<Type> TCargs;
    std::string abstractDefTypeStr = "(";
    if (nbArg > 0) {
        for (size_t i = 0; i < nbArg - 1; ++i) {
            std::string argName = "A";
            argName += std::to_string(i);

            abstractDefTypeStr += argName + ", ";
            TCargs.push_back(cmp.parseType(argName));
        }
        abstractDefTypeStr += std::string("A") + std::to_string(nbArg - 1) + ")->R";
        TCargs.push_back(cmp.parseType(std::string("A") + std::to_string(nbArg - 1)));
    } else {
        abstractDefTypeStr += ")->R";
    }

    TCargs.push_back(cmp.parseType("R"));

    Type abstractDefType = cmp.parseType(abstractDefTypeStr);
    Type funcClass = cmp.classBuilder(name).setAbstract(true).addAbstractDef("()", abstractDefType).build();
    Type funcTC = cmp.typeConstructorBuilder(name).setArgs(TCargs).setReturn(funcClass).build();

    dest.typeDef(name, funcTC);
}

void buildSTDModules(Compiler& cmp, ProgramBuilder builder) {
    Module slang = builder.openModule("sfsl").openModule("lang");
    slang.typeDef("unit", cmp.classBuilder("unit").build());
    slang.typeDef("bool", cmp.classBuilder("bool").build());
    slang.typeDef("int", cmp.classBuilder("int").build());
    slang.typeDef("real", cmp.classBuilder("real").build());
    slang.typeDef("string", cmp.classBuilder("string").build());

    createFunctionClass(cmp, slang, 0);
    createFunctionClass(cmp, slang, 1);
    createFunctionClass(cmp, slang, 2);
    createFunctionClass(cmp, slang, 3);
    createFunctionClass(cmp, slang, 4);
    createFunctionClass(cmp, slang, 5);
}

}

}
