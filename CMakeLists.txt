cmake_minimum_required(VERSION 3.5)
project(dear_imgui_im_sfml)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
        ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra")

find_package(SFML 2.3 COMPONENTS graphics window system REQUIRED)
find_package(GLEW)

include_directories(${SFML_INCLUDE_DIR})
include_directories(vendor/imgui)

add_library(imgui OBJECT
        vendor/imgui/imgui.cpp
        vendor/imgui/imgui_draw.cpp
        vendor/imgui/imgui_demo.cpp
        )

set(SOURCE_FILES
        imgui_impl_sfml_gl.cpp
        imgui_impl_sfml_gl.h
        main.cpp)

add_executable(dear_imgui_im_sfml ${SOURCE_FILES} $<TARGET_OBJECTS:imgui>)
target_link_libraries(dear_imgui_im_sfml ${SFML_LIBRARIES})
target_link_libraries(dear_imgui_im_sfml ${SFML_DEPENDENCIES})
target_link_libraries(dear_imgui_im_sfml ${GLEW_LIBRARIES} GL)
