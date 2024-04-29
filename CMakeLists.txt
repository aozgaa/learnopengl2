cmake_minimum_required(VERSION 3.27)

project(hello_opengl)

set(CMAKE_CXX_STANDARD 23)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

function(add_executable_learnopengl2 name)
    add_executable(${name} src/${name}.cpp)
    target_link_libraries(${name} PRIVATE glfw)
    target_link_libraries(${name} PRIVATE glad::glad)
endfunction()

set(LEARNOPENGL2_APPS
    1.1.hello_window
    1.2.hello_triangle
    1.2.hello_triangle_ebo
    1.2.hello_triangle_ex1
    1.2.hello_triangle_ex2
    1.2.hello_triangle_ex3
)

foreach(APP ${LEARNOPENGL2_APPS})
    add_executable_learnopengl2(${APP})
endforeach(APP)
