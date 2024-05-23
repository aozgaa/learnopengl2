cmake_minimum_required(VERSION 3.27)

project(hello_opengl)

add_definitions(-D_CRT_SECURE_NO_WARNINGS) # ignore windows deprecated functions
set(CMAKE_CXX_STANDARD 23)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

find_package(Stb REQUIRED)

add_custom_target(lint-all)

function(add_lint_target filename)
    get_filename_component(basename ${filename} NAME_WLE)
    add_custom_target(lint-${basename}
        COMMAND clang-format -i ${filename}
        COMMAND clang-tidy -p ${CMAKE_BINARY_DIR} ${filename}
    )
    add_dependencies(lint-all lint-${basename})
endfunction()

add_lint_target(${CMAKE_CURRENT_SOURCE_DIR}/src/include/file.h)
add_lint_target(${CMAKE_CURRENT_SOURCE_DIR}/src/include/image.h)
add_lint_target(${CMAKE_CURRENT_SOURCE_DIR}/src/include/shader_program.h)
add_lint_target(${CMAKE_CURRENT_SOURCE_DIR}/src/include/camera.h)

function(add_executable_learnopengl2 name)
    add_executable(${name} src/${name}.cpp)
    target_link_libraries(${name} PRIVATE glfw)
    target_link_libraries(${name} PRIVATE glad::glad)
    target_link_libraries(${name} PRIVATE glm::glm-header-only)
    target_include_directories(${name} PRIVATE src/include)
    target_include_directories(${name} PRIVATE ${Stb_INCLUDE_DIR})

    add_lint_target(${CMAKE_CURRENT_SOURCE_DIR}/src/${name}.cpp)
endfunction()

set(LEARNOPENGL2_APPS
    1.1.1.hello_window

    1.2.1.hello_triangle
    1.2.2.hello_triangle_ebo
    1.2.3.hello_triangle_ex1
    1.2.4.hello_triangle_ex2
    1.2.5.hello_triangle_ex3

    1.3.1.shaders_from_file
    1.3.2.shaders_vertex_color
    1.3.3.shaders_uniform
    1.3.4.shaders_color_attr
    1.3.5.shaders_ex1
    1.3.6.shaders_ex2
    # 1.3.shaders_ex3 # same as 1.3.shaders_vertex_color

    1.4.1.textures_triangle
    1.4.2.textures_square
    1.4.3.textures_2_texture_units
    # 1.4.textures_2_ex1 # same as 1.4.textures_2_texture_units
    1.4.5.textures_ex2
    1.4.6.textures_ex3
    
    1.5.1.transformations_translate
    1.5.2.transformations_scale_rotate
    1.5.3.transformations_time_corner
    1.5.4.transformations_ex1
    1.5.5.transformations_ex2

    1.6.1.coordinates_3d
    1.6.2.coordinates_cube
    1.6.3.coordinates_cube_zbuffer
    1.6.4.coordinates_cubes
    1.6.5.coordinates_cubes_whisky # generate lots of cubes randomly
    1.6.6.coordinates_ex1 # use mouse
    1.6.7.coordinates_ex2 # use mouse
    1.6.8.coordinates_ex3

    1.7.1.camera_lookat
    1.7.2.camera_rotate
    1.7.3.camera_keyboard
    1.7.4.camera_mouse
    1.7.5.camera_scroll
    1.7.6.camera_class
    1.7.7.camera_ex1
    1.7.8.camera_ex2
)

foreach(APP ${LEARNOPENGL2_APPS})
    add_executable_learnopengl2(${APP})
endforeach(APP)
