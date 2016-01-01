#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>
#include <memory>
#include "unistd.h"
#include "sfsl.h"

#define DEFAULT_CHUNK_SIZE 2048

using namespace sfsl;

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

int main(int argc, char** argv) {
    // LOAD FILE

    char* sourceFile = NULL;
    bool checkOnly = false;
    int option;

    while((option = getopt(argc, argv, "s:c")) != -1){
        switch (option) {
        case 's':
            sourceFile = optarg;
            break;
        case 'c':
            checkOnly = true;
            break;
        default:
            std::cerr << "unexpected program argument : " << option << std::endl;
        }
    }

    if (!sourceFile)
        sourceFile = (char*)"Examples\\test.sfsl";

    std::string source;

    std::ifstream f(sourceFile);
    while (f.good()) {
        source += f.get();
    }

    clock_t exec = clock();

    Compiler cmp(CompilerConfig(StandartReporter::CerrReporter));
    Pipeline ppl = Pipeline::createDefault();

    ByteCodeCollector bcc;
    EmptyCollector emc;
    AbstractOutputCollector* col;

    try {
        ProgramBuilder prog = cmp.parse(sourceFile, source);

        Module slang = prog.openModule("sfsl").openModule("lang");
        slang.typeDef("unit", cmp.classBuilder("unit").build());
        slang.typeDef("bool", cmp.classBuilder("bool").build());
        slang.typeDef("int", cmp.classBuilder("int").build());
        slang.typeDef("real", cmp.classBuilder("real").build());
        slang.typeDef("string", cmp.classBuilder("string").build());
/*
        Type R = cmp.parseType("R");
        Type Func0Class = cmp.classBuilder("Func0").setAbstract(true).addAbstractDef("()", cmp.parseType("()->R")).build();
        slang.typeDef("Func0", cmp.typeConstructorBuilder("Func0").setArgs({R}).setReturn(Func0Class).build());
*/

        createFunctionClass(cmp, slang, 0);
        createFunctionClass(cmp, slang, 1);
        createFunctionClass(cmp, slang, 2);
        createFunctionClass(cmp, slang, 3);
        createFunctionClass(cmp, slang, 4);

        if (checkOnly) {
            col = &emc;
            ppl.insert(Phase::StopRightBefore("CodeGen"));
        } else {
            col = &bcc;
        }

        cmp.compile(prog, *col, ppl);

        if (!checkOnly) {
            for (const std::string& i : bcc.get()) {
                std::cout << i << std::endl;
            }

            std::cout << "Compilation Time : " << (clock() - exec)/(double)CLOCKS_PER_SEC << std::endl << std::endl;
        }

    } catch(const CompileError& ex) {
        std::cerr << ex.what() << std::endl;
    }
}
