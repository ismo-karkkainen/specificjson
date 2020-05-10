cmake_minimum_required(VERSION 3.12)
project(specificjsontest CXX)

# This file is only for building test programs and programs for profiling
# the source code.

#### Parameter set-up for test programs

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
        set(BuildOptions -Wall -O2)
    else()
        set(BuildOptions -Wall -O0)
    endif()
    set(AlwaysSmallOptions -Os)
    set(ProfilerOptions -fprofile-instr-generate)
    set(ProfilerLinkOptions -fprofile-instr-generate)
    set(CxxStd -std=c++17)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
        set(BuildOptions -Wall -O2)
    else()
        set(BuildOptions -Wall -O0)
    endif()
    set(AlwaysSmallOptions -Os)
    set(ProfilerOptions -pg)
    set(ProfilerLinkOptions -pg)
    set(CxxStd -std=c++17)
else()
    set(BuildOptions "")
endif()

if (UNIX AND NOT APPLE)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

add_custom_command(OUTPUT specificjson
    COMMENT "Generating specificjson script"
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/build.yaml ${CMAKE_CURRENT_LIST_DIR}/build
    COMMAND ${CMAKE_CURRENT_LIST_DIR}/build --import ${CMAKE_CURRENT_LIST_DIR}/build.yaml --build specificjson)

add_custom_command(OUTPUT specificjson.hpp specificjson.cpp
    COMMENT "Generating specificjson script"
    DEPENDS specificjson ${CMAKE_CURRENT_LIST_DIR}/test/unittest.yaml
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/specificjson --input ${CMAKE_CURRENT_LIST_DIR}/test/unittest.yaml)

#### Tests

enable_testing()

add_executable(unittest test/unittest.cpp ${CMAKE_CURRENT_BINARY_DIR}/specificjson.cpp)
target_include_directories(unittest SYSTEM PRIVATE /usr/local/include)
target_include_directories(unittest SYSTEM PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_options(unittest PRIVATE ${CxxStd})
target_compile_options(unittest PRIVATE ${BuildOptions})
add_test(NAME UnitTest COMMAND unittest)

function(add_test_prog PROG)
    add_executable(${PROG} IMPORTED)
    set_property(TARGET ${PROG} PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/test/${PROG})
endfunction()

function(new_test TEST_NAME PROG)
    add_test(NAME ${TEST_NAME} COMMAND ${PROG})
    set_property(TEST ${TEST_NAME} PROPERTY ENVIRONMENT "PATH=${CMAKE_CURRENT_BINARY_DIR}:$ENV{PATH}")
endfunction()

function(new_test1 TEST_NAME PROG TST)
    add_test(NAME ${TEST_NAME} COMMAND ${PROG} ${CMAKE_CURRENT_LIST_DIR}/test/${TST})
    set_property(TEST ${TEST_NAME} PROPERTY ENVIRONMENT "PATH=${CMAKE_CURRENT_BINARY_DIR}:$ENV{PATH}")
endfunction()

function(new_test2 TEST_NAME PROG TST A2)
    add_test(NAME ${TEST_NAME} COMMAND ${PROG} ${CMAKE_CURRENT_LIST_DIR}/test/${TST} ${A2})
    set_property(TEST ${TEST_NAME} PROPERTY ENVIRONMENT "PATH=${CMAKE_CURRENT_BINARY_DIR}:${CMAKE_CURRENT_LIST_DIR}/test:$ENV{PATH}")
endfunction()

add_test_prog(clean.sh)
new_test(clean clean.sh)
add_test_prog(miss_req.sh)
new_test1(missing_requirement_yaml miss_req.sh miss_req.yaml)
new_test1(missing_requirement_json miss_req.sh miss_req.json)
add_test_prog(spec.sh)
new_test1(missing_referred_file spec.sh spec_missing_header.yaml)
new_test1(missing_spec_file spec.sh spec_missing_spec.yaml)
add_test_prog(list.sh)
new_test1(list list.sh ../build.yaml)
add_test_prog(standalone.sh)
new_test2(standalone standalone.sh standalone.yaml standalone)

#### Programs for profiling the code.

set(Profilers readfloatarray readfloatarray2 readstringarray readintarray)
add_custom_target(profile COMMENT "Building programs to be run with profiler")
add_dependencies(profile ${Profilers})
set(Readers profile/BlockQueue.cpp profile/InputChannel.cpp profile/FileDescriptorInput.cpp specificjson.cpp)

# Custom target that generates all profiler parsers at the same time.
list(TRANSFORM Profilers APPEND "_io.hpp" OUTPUT_VARIABLE GenHpp)
add_custom_target(generated
    COMMENT "Generating profiler types from profile/profile.md"
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/profile/profile.md
    COMMAND edicta -i ${CMAKE_CURRENT_LIST_DIR}/profile/profile.md -o specs ${Profilers}
    COMMAND ${CMAKE_CURRENT_LIST_DIR}/specificjson -i specs
    BYPRODUCTS ${GenHpp})

function(setup_profiling TGTNAME)
    add_executable(${TGTNAME} EXCLUDE_FROM_ALL profile/readparse.cpp ${Readers})
    add_dependencies(${TGTNAME} generated)
    target_compile_definitions(${TGTNAME} PRIVATE HEADER=${TGTNAME}_io.hpp)
    target_include_directories(${TGTNAME} SYSTEM PRIVATE /usr/local/include)
    target_include_directories(${TGTNAME} PRIVATE src)
    target_include_directories(${TGTNAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    target_compile_options(${TGTNAME} PRIVATE ${CxxStd})
    target_compile_options(${TGTNAME} PRIVATE ${BuildOptions} ${ProfilerOptions})
    target_link_libraries(${TGTNAME} PRIVATE ${ProfilerLinkOptions})
endfunction()

setup_profiling(readfloatarray)
setup_profiling(readfloatarray2)
setup_profiling(readstringarray)
setup_profiling(readintarray)
