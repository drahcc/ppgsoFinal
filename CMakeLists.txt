# This CMake script is designed for CLion IDE and QTCreator projects on Windows
# It should find bundled binary dependencies automatically when using MinGW compiler
# On Linux and Mac the dependencies (glm, glew, glfw) need to be installed manually

cmake_minimum_required(VERSION 3.5)
project(ppgso CXX)

#
# CONFIGURATION
#

# Basic CMake settings
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/_install)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)

# Use add_resources function to convert .glsl to a .cpp files which will be compiled into a separate static library
include(add_resources)

#
# DEPENDENCIES
#

# Set up external dependencies for Windows users
if (MINGW)
  set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/dependencies/include/")
  set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/dependencies/lib/mingw")
elseif (MSVC)
  set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/dependencies/include/")

  # Edit by: Samuel Zaprazny
  # EQUAL & GREATER_EQUAL instead of == & >=
  # == & >= was causing errors in the latest Visual Studio
  if(MSVC_VERSION EQUAL 1500)
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/dependencies/lib/vc2015")
  elseif(MSVC_VERSION GREATER_EQUAL 1900)
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/dependencies/lib/vc2019")
  endif()
  set(USE_STRICT_COMPILE_WARNINGS OFF CACHE BOOL "" FORCE)
endif ()

# Warnings for Debug mode
option(USE_STRICT_COMPILE_WARNINGS "Use strict compilation warnings in debug mode." ON)
# These compile flags should apply for GCC and Clang compilers
set(STRICT_COMPILE_FLAGS "-Wpedantic -Wall -Wno-c++98-compat -Wextra -Wno-sign-conversion -Wno-unused-parameter")
if (USE_STRICT_COMPILE_WARNINGS)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${STRICT_COMPILE_FLAGS}")
endif ()

# Find required packages
find_package(GLFW3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)
find_package(OpenGL REQUIRED)

# Edit by: Samuel Zaprazny
# Finding ASSIMP
find_package(ASSIMP)

if (ASSIMP_FOUND)
  add_definitions(-DUSE_ASSIMP=1)
  
  if (WIN32 AND MINGW)
    set(ASSIMP_ROOT_DIR "${ASSIMP_ROOT_DIR}/bin/mingw/")
  elseif (WIN32 AND MSVC)
    set(ASSIMP_ROOT_DIR "${ASSIMP_ROOT_DIR}/bin/vc/")
  endif ()

  # Installing Assimp binaries to project binary directory if using WINDOWS
  if (WIN32)
    file(COPY ${ASSIMP_ROOT_DIR} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    install(DIRECTORY ${ASSIMP_ROOT_DIR} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
  endif ()

endif ()

# Optional packages
find_package(OpenMP)
if(OPENMP_FOUND)
  list(APPEND CMAKE_CXX_FLAGS ${OpenMP_CXX_FLAGS})
endif()

# Set default installation destination
if (NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "../_install")
endif ()

#
# Shaders / Materials
#

set(PPGSO_SHADER_SRC
        shader/color_vert.glsl shader/color_frag.glsl
        shader/convolution_vert.glsl shader/convolution_frag.glsl
        shader/diffuse_vert.glsl shader/diffuse_frag.glsl
        shader/texture_vert.glsl shader/texture_frag.glsl
        shader/underwater_vert.glsl shader/underwater_frag.glsl
        shader/water_vert.glsl shader/water_frag.glsl
        shader/skybox_vert.glsl shader/skybox_frag.glsl
        shader/postprocess_vert.glsl shader/postprocess_frag.glsl
        )
add_resources(shaders ${PPGSO_SHADER_SRC})

# PPGSO library
if (ASSIMP_FOUND)
  message(STATUS "Using ASSIMP object loader")
  add_library(ppgso STATIC
          ppgso/Mesh_Assimp.cpp
          ppgso/tiny_obj_loader.cpp
          ppgso/shader.cpp
          ppgso/image.cpp
          ppgso/image_bmp.cpp
          ppgso/image_raw.cpp
          ppgso/texture.cpp
          ppgso/window.cpp
  )
else ()
  message(STATUS "Using TINY object loader")
  add_library(ppgso STATIC
          ppgso/Mesh_Tiny.cpp
          ppgso/tiny_obj_loader.cpp
          ppgso/shader.cpp
          ppgso/image.cpp
          ppgso/image_bmp.cpp
          ppgso/image_raw.cpp
          ppgso/texture.cpp
          ppgso/window.cpp
  )
endif ()

# Make sure GLM uses radians and GLEW is a static library
target_compile_definitions(ppgso PUBLIC -DGLM_FORCE_RADIANS -DGLEW_STATIC)

# Edit by: Samuel Zaprazny
# Linking assimp library
if (ASSIMP_FOUND)
    # Link to GLFW, GLEW. OpenGL and ASSIMP
    target_link_libraries(ppgso PUBLIC ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${ASSIMP_LIBRARIES})
    install(TARGETS ppgso DESTINATION .)
else ()
  # Link to GLFW, GLEW. OpenGL without ASSIMP
  # Using Tiny Obj Loader
  target_link_libraries(ppgso PUBLIC ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${ASSIMP_LIBRARIES})
  install(TARGETS ppgso DESTINATION .)
endif ()

# Pass on include directories
target_include_directories(ppgso PUBLIC
        ppgso
        ${GLFW_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIRS}
        ${GLM_INCLUDE_DIRS}
        ${OPENGL_INCLUDE_DIRS}
        ${ASSIMP_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR})

#
# TARGETS
#

# gl9_scene
add_executable(gl9_scene
        gl9_scene/gl9_scene.cpp
        gl9_scene/object.cpp
        gl9_scene/scene.cpp
        gl9_scene/camera.cpp
        gl9_scene/asteroid.cpp
        gl9_scene/generator.cpp
        gl9_scene/player.cpp
        gl9_scene/projectile.cpp
        gl9_scene/explosion.cpp
        gl9_scene/space.cpp)
target_link_libraries(gl9_scene ppgso shaders)
install(TARGETS gl9_scene DESTINATION .)
add_custom_command(TARGET gl9_scene POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data/ ${CMAKE_CURRENT_BINARY_DIR})

# Underwater Scene - Final Project
add_executable(underwater_scene
        underwater/underwater_main.cpp
        underwater/underwater_scene.cpp
        underwater/underwater_object.cpp
        underwater/underwater_camera.cpp
        underwater/ground.cpp
        underwater/fish.cpp
        underwater/fish_fin.cpp
        underwater/bubble.cpp
        underwater/bubble_generator.cpp
        underwater/jellyfish.cpp
        underwater/seaweed.cpp
        underwater/seaweed_instanced.cpp
        underwater/rock.cpp
        underwater/fish1.cpp
        underwater/skybox.cpp
        underwater/water_surface.cpp)
target_link_libraries(underwater_scene ppgso shaders)
install(TARGETS underwater_scene DESTINATION .)
add_custom_command(TARGET underwater_scene POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data/ ${CMAKE_CURRENT_BINARY_DIR})

#
# INSTALLATION
#

file(COPY "data/" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
install(DIRECTORY data/ DESTINATION .)
