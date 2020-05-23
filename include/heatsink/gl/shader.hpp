#pragma once

#include <filesystem>
#include <string>

#include <heatsink/platform/gl.hpp>

namespace heatsink::gl {
	/**
	 * A single OpenGL shader stage. This class has the same semantics as
	 * `object` in terms of lifetime, but does not actually extend from it as
	 * it does not follow the OpenGL object model.
	 */
	class shader {
	public:
		/**
		 * Create a shader, loading its source code from the given file path. If
		 * the filename used for the source does not match the format specified
		 * by `to_stage()`, the stage must be manually specified or else an
		 * exception will be thrown.
		 */
		static shader from_file(const std::filesystem::path&, GLenum stage = GL_NONE);

	public:
		/**
		 * Create a shader directly from source code. The desired stage must be
		 * manually passed since there is no filename to deduce from. Note that
		 * a file path or ID may optionally be passed; this allows the user to
		 * see what source the error came from, but does not effect the actual
		 * creation of the shader.
		 */
		shader(const std::string&, GLenum stage, const std::string& from = "");

		// Like `object`, shaders can only be moved.
		shader(const shader&) = delete;
		shader(shader&&) noexcept;
		~shader();

		shader& operator =(const shader&) = delete;
		shader& operator =(shader&&) noexcept;

	public:
		/**
		 * Check if the shader instance is valid. A shader should be valid
		 * unless it has thrown an exception or been moved from. Calling any
		 * member functions on an invalid shader is undefined behavior, although
		 * it is likely to raise an assertion.
		 */
		bool is_valid() const;

		/**
		 * Retrieve the OpenGL `name` (the instance identifier) for this shader.
		 * Use with caution; the identifier is still managed by the instance,
		 * and changes made directly to the identifier may not be reflected.
		 * Note that an assertion is raised if `is_valid()` would equal `false`.
		 */
		GLuint get() const;

		/**
		 * Retrieve the type of this shader; the stage it was created with.
		 */
		GLenum get_stage() const;

	private:
		// Compile the given source into this shader.
		void compile(const std::string&, const std::string& from);

	private:
		// The OpenGL identifier for this shader.
		GLuint m_name;
		// The stage the shader was created with.
		GLenum m_stage;
	};
}
