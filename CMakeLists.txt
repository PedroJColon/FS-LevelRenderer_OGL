cmake_minimum_required(VERSION 3.10)

project(LevelRendererOGL)

# currently using unicode in some libraries on win32 but will change soon
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

if (UNIX AND NOT APPLE)
     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lX11 -lGL")
     find_package(X11)
     find_package(OpenGL)
     include_directories(${X11_INCLUDE_DIR})
     link_libraries(${X11_LIBRARIES})
     include_directories(${OPENGL_INCLUDE_DIRS})
     link_libraries(${OPENGL_LIBRARIES})
endif()

add_executable (LevelRendererOGL main.cpp renderer.h HelperFunctions.h model.h h2bParser.h ../VertexShader.glsl ../PixelShader.glsl ../GameLevel.txt)

set_source_files_properties( VertexShader.glsl PROPERTIES
        VS_SHADER_TYPE Vertex 
        VS_SHADER_MODEL 5.0
        VS_SHADER_ENTRYPOINT main
        VS_TOOL_OVERRIDE "None" 
	# Tip: Swap "None" for "FXCompile" to have them actually be compiled by VS.(Great for D3D11/12)
)

set_source_files_properties( PixelShader.glsl PROPERTIES
        VS_SHADER_TYPE Vertex 
        VS_SHADER_MODEL 5.0
        VS_SHADER_ENTRYPOINT main
        VS_TOOL_OVERRIDE "None" 
	# Tip: Swap "None" for "FXCompile" to have them actually be compiled by VS.(Great for D3D11/12)
)
