set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(glfw3  REQUIRED)
find_package(epoxy  REQUIRED)
find_package(glm    REQUIRED)

set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/heatsink")
add_library(heatsink
	"${SRC}/error_compile.cpp"
	"${SRC}/error_debug.cpp"
	"${SRC}/error_exception.cpp"
	"${SRC}/gl_attribute.cpp"
	"${SRC}/gl_buffer.cpp"
	"${SRC}/gl_pixel_format.cpp"
	"${SRC}/gl_program.cpp"
	"${SRC}/gl_shader.cpp"
	"${SRC}/gl_vertex_format.cpp"
	"${SRC}/platform_context.cpp"
	"${SRC}/platform_window.cpp"
	"${SRC}/traits_name.cpp"
)

target_compile_definitions(heatsink PRIVATE GLFW_INCLUDE_NONE)
target_include_directories(heatsink PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(heatsink
	PUBLIC
		OpenGL::GL
		epoxy
		glm
	PRIVATE
		glfw
)
