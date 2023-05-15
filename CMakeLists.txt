cmake_minimum_required(VERSION 3.11)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

project(TheTOXEngine VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

include_directories(Engine/vendor/stb)
include_directories(Engine/vendor/tinyobjloader)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h)
string(REGEX REPLACE "CMakeFiles/[^;]+;?" "" SOURCE_FILES "${SOURCE_FILES}")
#message(${SOURCE_FILES})

add_executable(TOXEngine ${SOURCE_FILES})
target_link_libraries(TOXEngine "glfw3;vulkan")
