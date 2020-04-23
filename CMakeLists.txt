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


#### Tests

enable_testing()

set(Sources src/SpecificJSON.cpp src/SpecificJSON.cpp)

add_executable(unittest src/unittest.cpp ${Sources})
target_include_directories(unittest SYSTEM PRIVATE /usr/local/include)
target_include_directories(unittest PRIVATE src)
target_compile_definitions(unittest PRIVATE TESTDOC_UNITTEST)
target_compile_options(unittest PRIVATE ${CxxStd})
target_compile_options(unittest PRIVATE ${BuildOptions})
add_test(NAME UnitTest COMMAND unittest)


#### Programs for profiling the code.

set(Profilers readfloatarray readfloatarray2 readstringarray readintarray)
add_custom_target(profile COMMENT "Building programs to be run with profiler")
add_dependencies(profile ${Profilers})
set(Readers profile/BlockQueue.cpp profile/InputChannel.cpp profile/FileDescriptorInput.cpp src/SpecificJSON.cpp)

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
