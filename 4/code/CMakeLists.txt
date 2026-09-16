cmake_minimum_required(VERSION 3.10)
project(Assignment4 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 独立构建时自动检索依赖
if(NOT OpenCV_FOUND AND NOT DEFINED OpenCV_LIBRARIES)
    find_package(OpenCV REQUIRED)
endif()

add_executable(Assignment4 main.cpp)

target_link_libraries(Assignment4 PRIVATE ${OpenCV_LIBRARIES})

target_include_directories(Assignment4 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)
