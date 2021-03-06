cmake_minimum_required(VERSION 3.2)

project(zlog)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#
# zlog library
#

set(
    SOURCE_FILES
        src/ctstring.hpp
        src/ctf_writer.hpp
        src/ctf_writer.cpp
        src/fmt_helpers.hpp
        src/log.hpp
        src/log.cpp
        src/format_parser.hpp
        src/reflection.hpp
        src/reflection.cpp
        src/type_name.hpp
        src/type_traits.hpp
        
        src/barectf/barectf.h
        src/barectf/barectf.c
        src/barectf/barectf-platform-linux-fs.h
        src/barectf/barectf-platform-linux-fs.c
)

# Show folders in IDE
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" FILES ${SOURCE_FILES})

add_library(zlog ${SOURCE_FILES})

target_include_directories(
    zlog
        PUBLIC
        src
        src/barectf
        thirdparty
        thirdparty/magic_enum/include
)

#
# tests
#

# catch2 single header
add_library(catch INTERFACE)

target_include_directories(
    catch
        INTERFACE
        thirdparty/catch2
)

set(
    TEST_FILES
        test/test_ctf_writer.cpp
        test/test_format_parser.cpp
        test/test_log.cpp
        test/test_type_name.cpp
        test/test_type_traits.cpp
)

# Show folders in IDE
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/test" FILES ${TEST_FILES})

add_executable(unit_tests ${TEST_FILES})

target_link_libraries(
    unit_tests
        catch
        zlog
)

# Load and use the .cmake file provided by Catch so all the test cases
# are made available to CTest.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/catch2/cmake)
include(Catch)
include(CTest)
catch_discover_tests(unit_tests)
