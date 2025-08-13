# ImGui source files
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../external/imgui)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Find dependencies
find_package(OpenGL REQUIRED)
find_package(GLFW3 REQUIRED)

# Collect source files
file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glad.c
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sim/*.cpp
)

# Define the executable
add_executable(fluidSim
    ${SOURCE_FILES}
    ${IMGUI_SOURCES}
)

# Include directories
target_include_directories(fluidSim PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/sim
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/glm
    ${GLFW3_INCLUDE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Link libraries
target_link_libraries(fluidSim
    OpenGL::GL
    ${GLFW3_LIBRARY}
)
