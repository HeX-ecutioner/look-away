cmake_minimum_required(VERSION 3.16)
project(LookAway)

set(CMAKE_CXX_STANDARD 17)

# GLFW
add_subdirectory(external/glfw)

# GLAD
add_library(glad external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# ImGui
set(IMGUI_SRC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

add_executable(LookAway
    src/main.cpp
    src/App.cpp
    src/TimerManager.cpp
    src/UI.cpp
    ${IMGUI_SRC}
)

target_include_directories(LookAway PRIVATE
    include
    external/imgui
    external/imgui/backends
    external/glad/include
)

target_link_libraries(LookAway
    glfw
    glad
    opengl32
)