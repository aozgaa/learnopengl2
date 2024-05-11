cmake_minimum_required(VERSION 3.27)

project(hello_opengl)

set(CMAKE_CXX_STANDARD 23)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(Stb REQUIRED)

function(add_executable_learnopengl2 name)
    add_executable(${name} src/${name}.cpp)
    target_link_libraries(${name} PRIVATE glfw)
    target_link_libraries(${name} PRIVATE glad::glad)
    target_include_directories(${name} PRIVATE src/include)
    target_include_directories(${name} PRIVATE ${Stb_INCLUDE_DIR})
endfunction()

set(LEARNOPENGL2_APPS
    1.1.hello_window
    1.2.hello_triangle
    1.2.hello_triangle_ebo
    1.2.hello_triangle_ex1
    1.2.hello_triangle_ex2
    1.2.hello_triangle_ex3
    1.3.shaders_from_file
    1.3.shaders_vertex_color
    1.3.shaders_uniform
    1.3.shaders_color_attr
    1.3.shaders_ex1
    1.3.shaders_ex2
    # 1.3.shaders_ex3 # same as 1.3.shaders_vertex_color
    1.4.textures_triangle
    1.4.textures_square
    1.4.textures_2_texture_units
)

foreach(APP ${LEARNOPENGL2_APPS})
    add_executable_learnopengl2(${APP})
endforeach(APP)
