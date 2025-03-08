cmake_minimum_required(VERSION 3.15)
project(ParticleSimulation)

# Enable both C and C++
enable_language(C CXX)

# Set language standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add this line to suppress deprecation warnings
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# Find all source files using wildcards
file(GLOB_RECURSE SOURCES 
    "src/*.c"
    "src/*.cpp"
)

# Find all header files (optional, but good for IDE support)
file(GLOB_RECURSE HEADERS
    "inc/*.h"
    "inc/*.hpp"
)

# Add GLFW
add_subdirectory(external/glfw)

# Add GLAD
add_library(glad external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# Add cglm
add_subdirectory(external/cglm)

# Create ImGui library
add_library(imgui STATIC
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/imgui/backends
    ${CMAKE_SOURCE_DIR}/external/glfw/include
)

# Link ImGui with GLFW
target_link_libraries(imgui PUBLIC glfw)

# Add main executable
add_executable(main ${SOURCES} ${HEADERS})

# Set include directories
target_include_directories(main PRIVATE
    ${CMAKE_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/imgui/backends
    ${CMAKE_SOURCE_DIR}/external/glfw/include
    ${CMAKE_SOURCE_DIR}/external/glad/include
    ${CMAKE_SOURCE_DIR}/external/cglm/include
    ${CMAKE_SOURCE_DIR}/external/cgltf
    ${CMAKE_SOURCE_DIR}/external/stb
)

# Link all libraries in one command
target_link_libraries(main PRIVATE
    glfw
    glad
    cglm
    imgui
    opengl32
)

# Set the output directory for the executable
set_target_properties(main PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Make sure to copy shader files to the build directory
file(COPY assets/shaders DESTINATION ${CMAKE_SOURCE_DIR}/assets)