cmake_minimum_required(VERSION 3.10)
project(ClothSimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

set(EXTERNAL_INCLUDE "${CMAKE_SOURCE_DIR}/external/include")
set(EXTERNAL_LIB "${CMAKE_SOURCE_DIR}/external/lib")

set(SOURCES
    src/main.cpp
    src/cloth.cpp
    src/window_gl.cpp
    src/renderer.cpp
    src/shader.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${EXTERNAL_INCLUDE}
    ${OPENGL_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    ${EXTERNAL_LIB}/glfw3.lib
    ${EXTERNAL_LIB}/glew32s.lib
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
