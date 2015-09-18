project(SFSL)
cmake_minimum_required(VERSION 2.8)

# TARGET NAMES

set(COMPILER_LIB_TARGET         "libsfslc")
set(COMPILER_EXE_TARGET         "sfslc")
set(USER_API_TARGET             "sfsluser")

# TARGET OPTIONS

set(COMPILER_LIB_TYPE           STATIC)
set(COMPILER_LIB_OUTPUT_NAME    "sfslc")

# TARGET BUILD TYPES

set(COMPILER_LIB_BUILD_TYPE     debug)
set(COMPILER_EXE_BUILD_TYPE     release)
set(USER_API_BUILD_TYPE         debug)

###################################
#       UTILITY FUNCTIONS         #
###################################

function(apply_build_type target build_type)
    if(${build_type} STREQUAL debug)
        set_target_properties(${target} PROPERTIES COMPILE_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
    else()
        set_target_properties(${target} PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE} -o3")
    endif()
endfunction()

###################################
# COMPILER LIB BUILD INSTRUCTIONS #
###################################

file(GLOB_RECURSE COMPILER_SRC_LIST ./src/Compiler/*.cpp)
file(GLOB_RECURSE UTILS_SRC_LIST ./src/Utils/*.cpp)

add_library(${COMPILER_LIB_TARGET} ${COMPILER_LIB_TYPE} "${COMPILER_SRC_LIST}")
target_include_directories(${COMPILER_LIB_TARGET} PRIVATE ./src/)
set_target_properties(${COMPILER_LIB_TARGET} PROPERTIES OUTPUT_NAME ${COMPILER_LIB_OUTPUT_NAME})

apply_build_type(${COMPILER_LIB_TARGET} ${COMPILER_LIB_BUILD_TYPE})

###################################
# COMPILER EXE BUILD INSTRUCTIONS #
###################################

set(COMPILER_MAIN_FILE ./src/main.cpp)

add_executable(${COMPILER_EXE_TARGET} ${COMPILER_MAIN_FILE})
target_include_directories(${COMPILER_EXE_TARGET} PRIVATE ./src/)
target_link_libraries(${COMPILER_EXE_TARGET} ${COMPILER_LIB_TARGET})

apply_build_type(${COMPILER_EXE_TARGET} ${COMPILER_EXE_BUILD_TYPE})

################################
# MISCELLANEOUS
################################

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
