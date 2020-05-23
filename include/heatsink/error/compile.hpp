#pragma once

#include <ostream>
#include <string>

#include <heatsink/platform/gl.hpp>

namespace heatsink::gl {
	/**
	 * Format a shader error log and output it to a stream. This function should
	 * only be called when there is a shader error, that is, when the
	 * `GL_COMPILE_STATUS` of the shader is false. The `from` parameter is not
	 * required, but allows a source filename or other identifier to be included
	 * in each line of relevant output.
	 */
	void write_shader_log(std::ostream&, GLuint name, const std::string& from = "");
	/**
	 * Format a program error log and output it to a stream. This function
	 * should only be called when there is a program error, that is, when the
	 * `GL_LINK_STATUS` of the program is false.
	 */
	void write_program_log(std::ostream&, GLuint name, const std::string& from = "");
}
