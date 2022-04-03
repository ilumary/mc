cmake_minimum_required(VERSION 3.22)

project(MC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")

include_directories(include include/vulkan-core extern/stb extern/ktx/include)

file(GLOB SOURCES "src/*.cpp" "src/vulkan-core/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

add_subdirectory(extern/vk-bootstrap extern/vma)
add_subdirectory(extern/ktx)

target_include_directories(${PROJECT_NAME}
                PUBLIC /opt/homebrew/include
                PUBLIC /Library/Vulkan3/macOS/include
                PUBLIC extern/vk-bootstrap/srci
                PUBLIC extern/vma/include
                PUBLIC extern/ktx/include
)

target_link_directories(${PROJECT_NAME}
                PUBLIC /opt/homebrew/lib
                PUBLIC extern/ktx/lib
                PUBLIC /Library/Vulkan3/macOS/lib
                PRIVATE extern/vk-bootstrap/src
)

target_link_libraries(${PROJECT_NAME} glfw fmt vk-bootstrap ktx vulkan)
