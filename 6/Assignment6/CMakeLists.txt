cmake_minimum_required(VERSION 3.10)
project(Assignment6 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Assignment6
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

target_include_directories(Assignment6 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

