#include <iostream>
#include "sfsl.h"

const size_t MAX_FUNCTION_CLASS_COUNT = 10;

using namespace sfsl;

void createFunctionClass(ProgramBuilder builder, Module dest, size_t nbArg) {
    std::string name = "Func";
    name += std::to_string(nbArg);

    std::vector<Type> TCargs;
    std::string abstractDefTypeStr = "(";
    if (nbArg > 0) {
        for (size_t i = 0; i < nbArg - 1; ++i) {
            std::string argName = "A";
            argName += std::to_string(i);

            abstractDefTypeStr += argName + ", ";
            TCargs.push_back(builder.parseType(argName));
        }
        abstractDefTypeStr += std::string("A") + std::to_string(nbArg - 1) + ")->R";
        TCargs.push_back(builder.parseType(std::string("A") + std::to_string(nbArg - 1)));
    } else {
        abstractDefTypeStr += ")->R";
    }

    TCargs.push_back(builder.parseType("R"));

    Type abstractDefType = builder.parseType(abstractDefTypeStr);
    Type funcClass = builder.classBuilder(name).setAbstract(true).addAbstractDef("()", abstractDefType).build();
    Type funcTC = builder.typeConstructorBuilder(name).setArgs(TCargs).setReturn(funcClass).build();

    dest.typeDef(name, funcTC);
}

COMPILE_PASS(ProgramBuilder builder, Pipeline&, const std::vector<std::string>& args) {
    Module slang = builder.openModule("sfsl").openModule("lang");
    slang.typeDef("unit", builder.classBuilder("unit").build());
    slang.typeDef("bool", builder.classBuilder("bool").build());
    slang.typeDef("int", builder.classBuilder("int").build());
    slang.typeDef("real", builder.classBuilder("real").build());
    slang.typeDef("string", builder.classBuilder("string").build());

    size_t funcClassToCreate = args.size() > 0 ? (size_t) std::atol(args[0].c_str()) : MAX_FUNCTION_CLASS_COUNT;

    for (size_t i = 0; i < std::min(funcClassToCreate, MAX_FUNCTION_CLASS_COUNT); ++i) {
        createFunctionClass(builder, slang, i);
    }
}
