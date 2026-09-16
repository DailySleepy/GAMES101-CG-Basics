cmake_minimum_required(VERSION 3.10)
project(Assignment7 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Assignment7
    main.cpp
    Object.hpp
    Vector.cpp
    Vector.hpp
    Sphere.hpp
    global.hpp
    Triangle.hpp
    Scene.cpp
    Scene.hpp
    Light.hpp
    AreaLight.hpp
    BVH.cpp
    BVH.hpp
    Bounds3.hpp
    Ray.hpp
    Material.hpp
    Intersection.hpp
    Renderer.cpp
    Renderer.hpp
)

target_include_directories(Assignment7 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 启用 OpenMP 多线程加速（Renderer.cpp 中包含 #pragma omp 并行渲染）
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(Assignment7 PRIVATE OpenMP::OpenMP_CXX)
endif()

