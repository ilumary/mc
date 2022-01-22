cmake_minimum_required(VERSION 3.22)

project(MC)

set(CMAKE_CXX_STANDARD 20)

include_directories(include include/vulkan-core)

file(GLOB SOURCES "src/*.cpp" "src/vulkan-core/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

add_subdirectory(extern/vk-bootstrap extern/vma)

target_include_directories(${PROJECT_NAME} 
                PUBLIC /opt/homebrew/include
                PUBLIC /Library/Vulkan/macOS/include 
                PUBLIC extern/vk-bootstrap/src
                PUBLIC extern/vma/include
)

target_link_directories(${PROJECT_NAME} 
                PUBLIC /opt/homebrew/lib  
                PUBLIC /Library/Vulkan/macOS/lib
                PRIVATE extern/vk-bootstrap/src
)

target_link_libraries(${PROJECT_NAME} glfw fmt vk-bootstrap vulkan)