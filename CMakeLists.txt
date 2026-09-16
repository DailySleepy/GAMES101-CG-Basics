cmake_minimum_required(VERSION 3.10)
project(GAMES101 CXX)

# 统一配置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 构建选项，默认全部开启
option(BUILD_ASSIGNMENT_0 "Build Assignment 0" ON)
option(BUILD_ASSIGNMENT_1 "Build Assignment 1" ON)
option(BUILD_ASSIGNMENT_2 "Build Assignment 2" ON)
option(BUILD_ASSIGNMENT_3 "Build Assignment 3" ON)
option(BUILD_ASSIGNMENT_4 "Build Assignment 4" ON)
option(BUILD_ASSIGNMENT_5 "Build Assignment 5" ON)
option(BUILD_ASSIGNMENT_6 "Build Assignment 6" ON)
option(BUILD_ASSIGNMENT_7 "Build Assignment 7" ON)
option(BUILD_ASSIGNMENT_8 "Build Assignment 8" ON)

# 统一查找第三方依赖
message(STATUS "=============== GAMES101 Workspace Setup ===============")

# 1. 查找 Eigen3
find_package(Eigen3 REQUIRED)
if(Eigen3_FOUND)
    message(STATUS "Found Eigen3: ${EIGEN3_INCLUDE_DIR} (version: ${Eigen3_VERSION})")
    # 若 CMake 未自动生成 Eigen3::Eigen 目标，则创建接口目标以保持兼容
    if(NOT TARGET Eigen3::Eigen)
        add_library(Eigen3::Eigen INTERFACE IMPORTED)
        set_target_properties(Eigen3::Eigen PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${EIGEN3_INCLUDE_DIRS}"
        )
    endif()
endif()

# 2. 查找 OpenCV
find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    message(STATUS "Found OpenCV: ${OpenCV_INCLUDE_DIRS} (version: ${OpenCV_VERSION})")
endif()

message(STATUS "=========================================================")

# 组织各个作业子项目 (0 ~ 8)
if(BUILD_ASSIGNMENT_0 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/0/CMakeLists.txt")
    message(STATUS "Adding Subdirectory: Assignment 0")
    add_subdirectory(0)
endif()

if(BUILD_ASSIGNMENT_1 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/1/代码框架/CMakeLists.txt")
    message(STATUS "Adding Subdirectory: Assignment 1")
    add_subdirectory("1/代码框架" 1)
endif()

if(BUILD_ASSIGNMENT_2 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/2/代码框架/CMakeLists.txt")
    message(STATUS "Adding Subdirectory: Assignment 2")
    add_subdirectory("2/代码框架" 2)
endif()

if(BUILD_ASSIGNMENT_3 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/3/Code/CMakeLists.txt")
    message(STATUS "Adding Subdirectory: Assignment 3")
    add_subdirectory("3/Code" 3)
endif()

if(BUILD_ASSIGNMENT_4 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/4/code/CMakeLists.txt")
    message(STATUS "Adding Subdirectory: Assignment 4")
    add_subdirectory("4/code" 4)
endif()

if(BUILD_ASSIGNMENT_5 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/5/Code/CMakeLists.txt")
    message(STATUS "Adding Subdirectory: Assignment 5")
    add_subdirectory("5/Code" 5)
endif()

if(BUILD_ASSIGNMENT_6 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/6/Assignment6/CMakeLists.txt")
    message(STATUS "Adding Subdirectory: Assignment 6")
    add_subdirectory("6/Assignment6" 6)
endif()

if(BUILD_ASSIGNMENT_7 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/7/Assignment7/CMakeLists.txt")
    message(STATUS "Adding Subdirectory: Assignment 7")
    add_subdirectory("7/Assignment7" 7)
endif()

if(BUILD_ASSIGNMENT_8 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/8/CMakeLists.txt")
    find_package(Freetype QUIET)
    if(FREETYPE_FOUND)
        message(STATUS "Adding Subdirectory: Assignment 8")
        add_subdirectory(8)
    else()
        message(STATUS "Assignment 8 is skipped: Freetype not found in current environment. (Install Freetype or configure Freetype_DIR to enable Assignment 8)")
    endif()
endif()
