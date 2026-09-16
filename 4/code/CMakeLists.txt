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

if(COMMAND setup_opencv_runtime)
    setup_opencv_runtime(Assignment4)
elseif(WIN32 AND OpenCV_FOUND)
    set(OPENCV_BIN_DIR "${OpenCV_DIR}/${OpenCV_ARCH}/${OpenCV_RUNTIME}/bin")
    if(EXISTS "${OPENCV_BIN_DIR}")
        set_target_properties(Assignment4 PROPERTIES
            VS_DEBUGGER_ENVIRONMENT "PATH=${OPENCV_BIN_DIR};%PATH%"
        )
        add_custom_command(TARGET Assignment4 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<$<CONFIG:Debug>:${OPENCV_BIN_DIR}/opencv_world${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}0d.dll>$<$<NOT:$<CONFIG:Debug>>:${OPENCV_BIN_DIR}/opencv_world${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}0.dll>"
                "$<TARGET_FILE_DIR:Assignment4>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${OPENCV_BIN_DIR}/opencv_videoio_ffmpeg${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}0_64.dll"
                "$<TARGET_FILE_DIR:Assignment4>"
        )
    endif()
endif()
