project(SFSL)
cmake_minimum_required(VERSION 2.8)

aux_source_directory(./ SRC_LIST)
aux_source_directory(./Compiler/ SRC_LIST)
aux_source_directory(./Compiler/Common/ SRC_LIST)
aux_source_directory(./Compiler/Lexer/ SRC_LIST)
aux_source_directory(./Compiler/Lexer/Tokens/ SRC_LIST)
aux_source_directory(./Compiler/Parser/ SRC_LIST)
aux_source_directory(./Compiler/AST/ SRC_LIST)
aux_source_directory(./Compiler/AST/Nodes/ SRC_LIST)
aux_source_directory(./Compiler/AST/Symbols/ SRC_LIST)
aux_source_directory(./Compiler/AST/Visitors/ SRC_LIST)
aux_source_directory(./Compiler/Analyser/ SRC_LIST)
aux_source_directory(./Compiler/Types/ SRC_LIST)

aux_source_directory(./Interpreter/ SRC_LIST)

aux_source_directory(./Utils/ SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST})

set(CMAKE_BUILD_TYPE "release")

set(CMAKE_CXX_FLAGS "-std=c++11 -o3 ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
