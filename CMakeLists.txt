
cmake_minimum_required(VERSION 3.1)

project(cpp17_headers CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS off)

include_directories(include)

add_subdirectory(test)



