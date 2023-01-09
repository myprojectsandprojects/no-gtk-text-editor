cmake_minimum_required(VERSION 3.5.1)

project(MyGlfwTest)
add_executable(MyGlfwTest main.cpp)

set(CMAKE_BUILD_TYPE Debug)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

#add_subdirectory(glfw-3.3.8/ build/)
add_subdirectory(glfw-3.3.8/)
target_link_libraries(MyGlfwTest glfw)
target_include_directories(MyGlfwTest PUBLIC glfw-3.3.8/include)

#target_link_libraries(MyGlfwTest PUBLIC /home/eero/all/no-gtk-text-editor/glfw/test/glfw-3.3.8/build/src/libglfw3.a)
#target_link_libraries(MyGlfwTest PUBLIC X11)
#target_link_libraries(MyGlfwTest PUBLIC dl)
#target_link_libraries(MyGlfwTest PUBLIC pthread)
