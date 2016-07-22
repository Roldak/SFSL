#include <iostream>
#include "sfsl.h"

const size_t MAX_FUNCTION_CLASS_COUNT = 10;

using namespace sfsl;

void createFunctionClass(ProgramBuilder builder, Module dest, size_t nbArg) {
    std::string name = "Func";
    name += std::to_string(nbArg);

    std::vector<TypeConstructorBuilder::Parameter> TCargs;
    std::string abstractDefTypeStr = "(";
    if (nbArg > 0) {
        for (size_t i = 0; i < nbArg - 1; ++i) {
            std::string argName = "A";
            argName += std::to_string(i);

            abstractDefTypeStr += argName + ", ";
            TCargs.push_back({argName, TypeConstructorBuilder::Parameter::V_IN});
        }

        std::string argName = "A";
        argName += std::to_string(nbArg - 1);

        abstractDefTypeStr += argName;
        TCargs.push_back({argName, TypeConstructorBuilder::Parameter::V_IN});
    }

    abstractDefTypeStr += ")->R";

    TCargs.push_back({"R", TypeConstructorBuilder::Parameter::V_OUT});

    Type abstractDefType = builder.parseType(abstractDefTypeStr);
    Type funcClass = builder.classBuilder(name).setAbstract(true).addAbstractDef("()", abstractDefType).build();
    Type funcTC = builder.typeConstructorBuilder(name).setParams(TCargs).setReturn(funcClass).build();

    dest.typeDef(name, funcTC);
}

COMPILE_PASS(ProgramBuilder builder, Pipeline&, const std::vector<std::string>& args) {
    Module slang = builder.openModule("sfsl").openModule("lang");
    slang.typeDef("unit", builder.classBuilder("unit").build());
    slang.typeDef("bool", builder.classBuilder("bool").build());
    slang.typeDef("int", builder.classBuilder("int").build());
    slang.typeDef("real", builder.classBuilder("real").build());
    slang.typeDef("string", builder.classBuilder("string").build());

    slang.typeDef("Box",
      builder.typeConstructorBuilder("Box")
        .setParams({TypeConstructorBuilder::Parameter("T")})
        .setReturn(builder.classBuilder("Box")
                    .addField("value", builder.parseType("T"))
                    .build()).build());

    size_t funcClassToCreate = args.size() > 0 ? (size_t) std::atol(args[0].c_str()) : MAX_FUNCTION_CLASS_COUNT;

    for (size_t i = 0; i < std::min(funcClassToCreate, MAX_FUNCTION_CLASS_COUNT); ++i) {
        createFunctionClass(builder, slang, i);
    }
}
