cmake_minimum_required(VERSION 3.10)
project(Assignment0 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 独立构建时自动检索依赖
if(NOT TARGET Eigen3::Eigen)
    find_package(Eigen3 REQUIRED)
    if(NOT TARGET Eigen3::Eigen)
        add_library(Eigen3::Eigen INTERFACE IMPORTED)
        set_target_properties(Eigen3::Eigen PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${EIGEN3_INCLUDE_DIRS}"
        )
    endif()
endif()

add_executable(Assignment0 main.cpp)

target_link_libraries(Assignment0 PRIVATE Eigen3::Eigen)

if(DEFINED EIGEN3_INCLUDE_DIRS)
    target_include_directories(Assignment0 PRIVATE ${EIGEN3_INCLUDE_DIRS})
endif()
