cmake_minimum_required(VERSION 3.10)

project(Task)

set(CMAKE_CXX_STANDARD 23)

find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(DevIL REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTASK_ROOT_PATH=\\\"\"${CMAKE_CURRENT_LIST_DIR}\\\"\" -D_CRT_SECURE_NO_WARNINGS")

add_executable(Task ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp ${CMAKE_CURRENT_LIST_DIR}/src/shader_util.cpp ${CMAKE_CURRENT_LIST_DIR}/src/texture_util.cpp)

target_include_directories(Task PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(Task PRIVATE glfw OpenGL::GL GLEW::GLEW glm::glm-header-only DevIL::IL DevIL::ILU DevIL::ILUT)