cmake_minimum_required(VERSION 3.10)
project(Assignment5 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Assignment5
    main.cpp
    Object.hpp
    Vector.hpp
    Sphere.hpp
    global.hpp
    Triangle.hpp
    Scene.cpp
    Scene.hpp
    Light.hpp
    Renderer.cpp
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(Assignment5 PRIVATE -Wall -Wextra -pedantic -Wshadow -Wreturn-type)
elseif(MSVC)
    target_compile_options(Assignment5 PRIVATE /W3)
endif()

target_include_directories(Assignment5 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(Assignment5 PRIVATE OpenMP::OpenMP_CXX)
endif()

set_target_properties(Assignment5 PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)


