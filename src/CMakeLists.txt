cmake_minimum_required (VERSION 2.6)
project (fire-framework)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

file(GLOB_RECURSE sources "${PROJECT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE headers "${PROJECT_SOURCE_DIR}/*.h")

include_directories("${PROJECT_SOURCE_DIR}/../lib/glew-1.10.0/include")
include_directories("${PROJECT_SOURCE_DIR}/../lib/glm-0.9.4.4/glm")
include_directories("${PROJECT_SOURCE_DIR}/../lib/freeglut-2.8.1/include")
include_directories("${PROJECT_SOURCE_DIR}/../lib/assimp-3.0.1270/include")
include_directories("${PROJECT_SOURCE_DIR}/../lib/SOIL/src")
include_directories("${PROJECT_SOURCE_DIR}/../lib/glsw")

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.53.0) 

if(Boost_FOUND)
    INCLUDE_DIRECTORIES("${Boost_INCLUDE_DIR}/..")                                                                                                                                                                                                                        
endif()

link_directories("${PROJECT_SOURCE_DIR}/../lib/freeglut-2.8.1/src")
link_directories("${PROJECT_SOURCE_DIR}/../lib/glew-1.10.0/lib")
link_directories("${PROJECT_SOURCE_DIR}/../lib/assimp-3.0.1270/lib")

add_library( libglut SHARED IMPORTED )
add_subdirectory (lib/SOIL)
add_subdirectory(lib/glsw)

add_executable (fire-framework 
	Camera.cpp
	Intersect.cpp
	Intersect.hpp
	Light.cpp
	LightManager.cpp
	Main.cpp
	Mesh.cpp
	Particles.cpp
	Renderable.cpp
	Scene.cpp
	Shader.cpp
	SH.cpp
	SHMat.cpp
	Texture.cpp
)

target_link_libraries( fire-framework ${Boost_LIBRARIES} glut GL GLEW X11 assimp SOIL glsw)
