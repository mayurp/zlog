cmake_minimum_required(VERSION 3.2)

project(zlog)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) 

add_executable(
	tests 
		test/test_ctf_writer.cpp
		test/test_format_parser.cpp

		src/barectf/barectf.c
		src/barectf/barectf-platform-linux-fs.c
		src/ctf_writer.cpp
		src/log.cpp
		src/reflection.cpp

)

target_include_directories(
    tests 
        PUBLIC 
        src 
        src/barectf
        thirdparty
        thirdparty/magic_enum/include
        thirdparty/catch2/single_include
)
