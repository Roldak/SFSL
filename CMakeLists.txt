project(SFSL)
cmake_minimum_required(VERSION 2.8)

aux_source_directory(./ SRC_LIST)
aux_source_directory(./Compiler/ SRC_LIST)
aux_source_directory(./Compiler/Common/ SRC_LIST)
aux_source_directory(./Compiler/Frontend/Lexer/ SRC_LIST)
aux_source_directory(./Compiler/Frontend/Lexer/Tokens/ SRC_LIST)
aux_source_directory(./Compiler/Frontend/Parser/ SRC_LIST)
aux_source_directory(./Compiler/Frontend/AST/ SRC_LIST)
aux_source_directory(./Compiler/Frontend/AST/Nodes/ SRC_LIST)
aux_source_directory(./Compiler/Frontend/AST/Symbols/ SRC_LIST)
aux_source_directory(./Compiler/Frontend/AST/Visitors/ SRC_LIST)
aux_source_directory(./Compiler/Frontend/Analyser/ SRC_LIST)
aux_source_directory(./Compiler/Frontend/Types/ SRC_LIST)
aux_source_directory(./Compiler/Frontend/Kinds/ SRC_LIST)
aux_source_directory(./Compiler/Backend/CodeGen/ SRC_LIST)
aux_source_directory(./Compiler/Backend/ SRC_LIST)
aux_source_directory(./Compiler/Backend/Bytecode SRC_LIST)

aux_source_directory(./Interpreter/ SRC_LIST)

aux_source_directory(./Utils/ SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST})

set(CMAKE_BUILD_TYPE "debug")

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
