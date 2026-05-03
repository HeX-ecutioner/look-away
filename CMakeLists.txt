cmake_minimum_required(VERSION 3.16)
project(LookAway VERSION 1.0.0 DESCRIPTION "A simple, minimal desktop app to help you rest your eyes.")
set(CMAKE_CXX_STANDARD 17)

# Pass -DDEBUG_TIMER=ON to cmake to use a 10s work / 5s break timer for testing.
# Default is OFF (20min / 20s — production behaviour).
option(DEBUG_TIMER "Use short debug timer (10s work, 5s break)" OFF)
if(DEBUG_TIMER)
    add_compile_definitions(DEBUG_TIMER)
endif()

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

# Resources (separate object library to avoid path issues with spaces in RC compiler)
add_library(LookAwayResources OBJECT assets/resources.rc)
set_target_properties(LookAwayResources PROPERTIES INCLUDE_DIRECTORIES "")

# WIN32 subsystem = no console window
add_executable(LookAway WIN32
    src/main.cpp
    src/App.cpp
    src/TimerManager.cpp
    src/TrayIcon.cpp
    src/UI.cpp
    $<TARGET_OBJECTS:LookAwayResources>
    ${IMGUI_SRC}
)

target_include_directories(LookAway PRIVATE
    include
    external/imgui
    external/imgui/backends
    external/glad/include
    external/glfw/include # for glfw3native.h
)

target_link_libraries(LookAway
    glfw
    glad
    opengl32
    shell32 # Shell_NotifyIcon
    user32 # Win32 window / menu APIs
    winmm  # PlaySound
)

# Copy font files next to the .exe so runtime paths resolve
file(GLOB FONT_FILES "${CMAKE_SOURCE_DIR}/external/imgui/misc/fonts/*.ttf")
foreach(FONT ${FONT_FILES})
    get_filename_component(FONT_NAME ${FONT} NAME)
    configure_file(${FONT} "${CMAKE_CURRENT_BINARY_DIR}/fonts/${FONT_NAME}" COPYONLY)
endforeach()