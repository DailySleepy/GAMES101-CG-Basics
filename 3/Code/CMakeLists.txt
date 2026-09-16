cmake_minimum_required(VERSION 3.10)
project(Assignment3 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 独立构建时自动检索依赖
if(NOT OpenCV_FOUND AND NOT DEFINED OpenCV_LIBRARIES)
    find_package(OpenCV REQUIRED)
endif()

if(NOT TARGET Eigen3::Eigen)
    find_package(Eigen3 REQUIRED)
    if(NOT TARGET Eigen3::Eigen)
        add_library(Eigen3::Eigen INTERFACE IMPORTED)
        set_target_properties(Eigen3::Eigen PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${EIGEN3_INCLUDE_DIRS}"
        )
    endif()
endif()

add_executable(Assignment3
    main.cpp
    rasterizer.hpp
    rasterizer.cpp
    global.hpp
    Triangle.hpp
    Triangle.cpp
    Texture.hpp
    Texture.cpp
    Shader.hpp
    OBJ_Loader.h
)

target_link_libraries(Assignment3 PRIVATE
    ${OpenCV_LIBRARIES}
    Eigen3::Eigen
)

target_include_directories(Assignment3 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIRS}
)

if(COMMAND setup_opencv_runtime)
    setup_opencv_runtime(Assignment3)
elseif(WIN32 AND OpenCV_FOUND)
    set(OPENCV_BIN_DIR "${OpenCV_DIR}/${OpenCV_ARCH}/${OpenCV_RUNTIME}/bin")
    if(EXISTS "${OPENCV_BIN_DIR}")
        set_target_properties(Assignment3 PROPERTIES
            VS_DEBUGGER_ENVIRONMENT "PATH=${OPENCV_BIN_DIR};%PATH%"
        )
        add_custom_command(TARGET Assignment3 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<$<CONFIG:Debug>:${OPENCV_BIN_DIR}/opencv_world${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}0d.dll>$<$<NOT:$<CONFIG:Debug>>:${OPENCV_BIN_DIR}/opencv_world${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}0.dll>"
                "$<TARGET_FILE_DIR:Assignment3>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${OPENCV_BIN_DIR}/opencv_videoio_ffmpeg${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}0_64.dll"
                "$<TARGET_FILE_DIR:Assignment3>"
        )
    endif()
endif()