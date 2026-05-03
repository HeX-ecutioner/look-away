cmake_minimum_required(VERSION 3.16)
project(LookAway VERSION 1.0.0 DESCRIPTION "A simple, minimal desktop app to help you rest your eyes.")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization and performance flags
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
    # Standard Release optimizations (without slow LTCG)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Oi /Ot /Oy /MT")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    # Aggressive optimizations for GCC/Clang (O3 is fast, flto is slow)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -s")
endif()

# Pass -DDEBUG_TIMER=ON to cmake to use a 10s work / 5s break timer for testing.
option(DEBUG_TIMER "Use short debug timer (10s work, 5s break)" OFF)
if(DEBUG_TIMER)
    add_compile_definitions(DEBUG_TIMER)
endif()

# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

# GLAD
add_library(glad STATIC external/glad/src/glad.c)
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

# Resources
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
    external/glfw/include
)

target_link_libraries(LookAway
    glfw
    glad
    opengl32
    shell32
    user32
    winmm
)

# Copy only the specific font files we actually use
set(REQUIRED_FONTS "Cousine-Regular.ttf" "Roboto-Medium.ttf")
foreach(FONT_NAME ${REQUIRED_FONTS})
    set(FONT_SRC "${CMAKE_SOURCE_DIR}/external/imgui/misc/fonts/${FONT_NAME}")
    if(EXISTS ${FONT_SRC})
        configure_file(${FONT_SRC} "${CMAKE_CURRENT_BINARY_DIR}/fonts/${FONT_NAME}" COPYONLY)
    endif()
endforeach()