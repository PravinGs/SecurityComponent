cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler and flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Werror -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")  # or use C++14 if needed

# Include directories
include_directories(include)

# Source files
file(GLOB SRCS "src/*.cpp")
file(GLOB TEST_SRCS "test/*.cpp")

# Object files
set(OBJ_DIR ${CMAKE_BINARY_DIR}/obj)
file(MAKE_DIRECTORY ${OBJ_DIR})

foreach(source_file ${SRCS})
    get_filename_component(source_file_name ${source_file} NAME_WE)
    set(obj_file ${OBJ_DIR}/${source_file_name}.o)
    add_executable(${source_file_name} ${source_file})
    target_link_libraries(${source_file_name} -lamqpcpp -lpthread -ldl -lev -ljsoncpp -lz -lcurl -lssl -lcrypto -lboost_system -lpugixml -lpcre2-8)
endforeach()

foreach(test_source_file ${TEST_SRCS})
    get_filename_component(test_source_file_name ${test_source_file} NAME_WE)
    set(obj_file ${OBJ_DIR}/${test_source_file_name}.o)
    add_executable(${test_source_file_name} ${test_source_file} ${SRCS})
    target_link_libraries(${test_source_file_name} -lamqpcpp -lpthread -ldl -lev -ljsoncpp -lz -lcurl -lssl -lcrypto -lboost_system -lpugixml -lpcre2-8)
endforeach()
