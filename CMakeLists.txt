project(SFSL)
cmake_minimum_required(VERSION 2.8)

###################################
#           PARAMETERS            #
###################################

# TARGET OUTPUT NAMES

set(COMPILER_LIB_OUTPUT_NAME    "sfslc") # will become libsfslc.a
set(USER_API_OUTPUT_NAME        "sfsl")
set(COMPILER_EXE_OUTPUT_NAME    "sfslc")
set(COMPILER_TESTS_OUTPUT_NAME  "sfsltests")

# TARGET OPTIONS

set(COMPILER_LIB_TYPE           STATIC) # recommended, as symbols are not marked __declspec(dllexport) yet
set(USER_API_LIB_TYPE           SHARED)

# TARGET BUILD TYPES

set(COMPILER_LIB_BUILD_TYPE     debug)
set(USER_API_BUILD_TYPE         debug)
set(COMPILER_EXE_BUILD_TYPE     debug)
set(COMPILER_TESTS_BUILD_TYPE   debug)

###################################
#       CMAKE DEFINITIONS         #
###################################

# TARGET IDENTIFIERS

set(COMPILER_LIB_TARGET         libsfslc)
set(USER_API_TARGET             sfsl)
set(COMPILER_EXE_TARGET         sfslc)
set(COMPILER_TESTS_TARGET       sfsltests)

function(apply_target_properties target build_type output_name)
    if(${build_type} STREQUAL debug)
        set_target_properties(${target} PROPERTIES COMPILE_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
        set_target_properties(${target} PROPERTIES OUTPUT_NAME "${output_name}-d")
    else()
        set_target_properties(${target} PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE} -o3")
        set_target_properties(${target} PROPERTIES OUTPUT_NAME ${output_name})
    endif()
endfunction()

###################################
# COMPILER LIB BUILD INSTRUCTIONS #
###################################

file(GLOB_RECURSE COMPILER_SRC_LIST ./src/Compiler/*.cpp)
file(GLOB_RECURSE UTILS_SRC_LIST ./src/Utils/*.cpp)

add_library(${COMPILER_LIB_TARGET} ${COMPILER_LIB_TYPE} ${COMPILER_SRC_LIST} ${UTILS_SRC_LIST})
apply_target_properties(${COMPILER_LIB_TARGET} ${COMPILER_LIB_BUILD_TYPE} ${COMPILER_LIB_OUTPUT_NAME})

target_include_directories(${COMPILER_LIB_TARGET} PRIVATE ./src/)

###################################
#   USER API BUILD INSTRUCTIONS   #
###################################

set(API_MAIN_FILE ./src/apimain.cpp)

file(GLOB_RECURSE API_SRC_LIST      ./src/Interface/*.cpp)
file(GLOB_RECURSE API_HEADERS_LIST  ./include/*.h)

add_library(${USER_API_TARGET} ${USER_API_LIB_TYPE} ${API_SRC_LIST} ${API_HEADERS_LIST})
apply_target_properties(${USER_API_TARGET} ${USER_API_BUILD_TYPE} ${USER_API_OUTPUT_NAME})

target_include_directories(${USER_API_TARGET} PRIVATE ./src/)
target_include_directories(${USER_API_TARGET} PRIVATE ./include/)
target_compile_definitions(${USER_API_TARGET} PRIVATE -DBUILD_API)
target_link_libraries(${USER_API_TARGET} ${COMPILER_LIB_TARGET})

###################################
# COMPILER EXE BUILD INSTRUCTIONS #
###################################

set(COMPILER_MAIN_FILE ./src/main.cpp)

add_executable(${COMPILER_EXE_TARGET} ${COMPILER_MAIN_FILE})
apply_target_properties(${COMPILER_EXE_TARGET} ${COMPILER_EXE_BUILD_TYPE} ${COMPILER_EXE_OUTPUT_NAME})

target_include_directories(${COMPILER_EXE_TARGET} PRIVATE ./src/)
target_link_libraries(${COMPILER_EXE_TARGET} ${COMPILER_LIB_TARGET})

#####################################
# COMPILER TESTS BUILD INSTRUCTIONS #
#####################################

set(COMPILER_TESTS_MAIN_FILE ./tests/main.cpp)

file(GLOB_RECURSE COMPILER_TESTS_SRC_LIST ./tests/*.cpp)

add_executable(${COMPILER_TESTS_TARGET} ${COMPILER_TESTS_SRC_LIST} ${COMPILER_TESTS_MAIN_FILE})
apply_target_properties(${COMPILER_TESTS_TARGET} ${COMPILER_TESTS_BUILD_TYPE} ${COMPILER_TESTS_OUTPUT_NAME})

target_include_directories(${COMPILER_TESTS_TARGET} PRIVATE ./src/)
target_include_directories(${COMPILER_TESTS_TARGET} PRIVATE ./include/)
target_link_libraries(${COMPILER_TESTS_TARGET} ${USER_API_TARGET})

###################################
#          MISCELLANEOUS          #
###################################

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
