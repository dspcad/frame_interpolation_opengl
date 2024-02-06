# CMake entry point
cmake_minimum_required (VERSION 3.0)
project (Frame_Interpolation)

find_package(OpenGL REQUIRED)
find_package(PNG REQUIRED)



if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
	message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
	message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()



# Compile external dependencies 
add_subdirectory (external)

cmake_policy(SET CMP0026 OLD)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/rpavlik-cmake-modules-fe2273")
include(CreateLaunchers)

if(INCLUDE_DISTRIB)
    add_subdirectory(distrib)
endif(INCLUDE_DISTRIB)



include_directories(
	${PNG_INCLUDE_DIR}
	external/AntTweakBar-1.16/include/
	external/glfw-3.1.2/include/
	external/glm-0.9.7.1/
	external/glew-1.13.0/include/
	external/assimp-3.0.1270/include/
	external/bullet-2.81-rev2613/src/
	.
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	${PNG_LIBRARY}
	glfw
	GLEW_1130
)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

# Weather Effect Demo
add_executable(weather_effect
	demo/weather_effect.cpp
	common/shader.cpp
	common/shader.hpp
	common/texture.cpp
	common/texture.hpp
	
	demo/SimpleFragmentShader.fragmentshader
	demo/SimpleVertexShader.vertexshader
)
target_link_libraries(weather_effect
	${ALL_LIBS}
)
# Xcode and Visual working directories
set_target_properties(weather_effect PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/demo/")
create_target_launcher(weather_effect WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/demo/")

# Weather Effect Demo
add_executable(img_render
	img_render/weather_effect.cpp
	common/shader.cpp
	common/shader.hpp
	common/texture.cpp
	common/texture.hpp
	common/img_util.cpp
	common/img_util.hpp
	
	img_render/SimpleFragmentShader.fragmentshader
	img_render/SimpleVertexShader.vertexshader
)
target_link_libraries(img_render
	${ALL_LIBS}
)
# Xcode and Visual working directories
set_target_properties(img_render PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/img_render/")
create_target_launcher(img_render WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/img_render/")

# simple compute shader
add_executable(compute_shader
        compute_shader/frame_interpolation.cpp
        common/shader.cpp
        common/shader.hpp
        common/texture.cpp
        common/texture.hpp

        compute_shader/compute.shader
)
target_link_libraries(compute_shader
        ${ALL_LIBS}
)
# Xcode and Visual working directories
set_target_properties(compute_shader PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/compute_shader/")
create_target_launcher(compute_shader WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/compute_shader/")

