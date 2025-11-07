cmake_minimum_required(VERSION 3.17)
project(GameOfLife VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG master)
FetchContent_Declare(glad GIT_REPOSITORY https://github.com/joey8angelo/Glad.git GIT_TAG main)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

FetchContent_MakeAvailable(glad)

include_directories(include include/imgui)

set(SRC_FILES 
  src/main.cpp
  src/imgui/imgui.cpp
  src/imgui/imgui_demo.cpp
  src/imgui/imgui_draw.cpp
  src/imgui/imgui_tables.cpp
  src/imgui/imgui_widgets.cpp
  src/imgui/imgui_impl_glfw.cpp
  src/imgui/imgui_impl_opengl3.cpp
)

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC "source")
target_link_libraries(${PROJECT_NAME} PRIVATE glad glfw)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror -O3)

add_executable(${PROJECT_NAME}_debug ${SRC_FILES})
target_include_directories(${PROJECT_NAME}_debug PUBLIC "source")
target_link_libraries(${PROJECT_NAME}_debug PRIVATE glad glfw)
target_compile_options(${PROJECT_NAME}_debug PRIVATE -Wall -Werror -g -O0)