#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include <heatsink/gl/attribute.hpp>
#include <heatsink/gl/shader.hpp>
#include <heatsink/platform/gl.hpp>

namespace heatsink::gl {
	/**
	 * An OpenGL program; a set of shaders linked to form a GPU executable. This
	 * class has the same semantics as `object` in terms of lifetime, but does
	 * not actually extend it as it does not follow the OpenGL object model.
	 */
	class program {
	public:
		/**
		 * Allow a shader object to be referenced inside of a collection, like
		 * when passing a list to the program constructor. This is copyable,
		 * storing the shader name without managing its lifetime. Note then,
		 * that the lifetime is tied to that of the shader it is created with.
		 */
		class shader_name;
	
	public:
		/**
		 * Construct a program from a set of file paths. Each path is used to
		 * create a shader that is then passed to the standard constructor. Note
		 * that each path must be named with an extension as specified in
		 * `shader::to_stage()`, there is no way to specify overrides like in
		 * the `shader::from_file()` constructor. The shaders are freed after
		 * construction of the program.
		 */
		static program from_files(const std::vector<std::filesystem::path>&);

	public:
		/**
		 * Construct a program from a set of pre-existing shader objects. Note
		 * that this function takes an initializer list of references such that
		 * the argument does not have to own the shaders / be moved into. Like
		 * `shader`, a file path or ID may optionally be passed to identify what
		 * shader set a linker error came from.
		 */
		program(const std::vector<shader_name>&, const std::string& from = "");

		// Like `object`, programs can only be moved.
		program(const program&) = delete;
		program(program&&) noexcept;
		~program();

		program& operator =(const program&) = delete;
		program& operator =(program&&) noexcept;

	private:
		// Create a program with the given shader names; this constructor is
		// utilized by the public variant and the static creation methods.
		program(const std::vector<GLuint>& names, const std::string& from);

	public:
		/**
		 * "Bind" the shader to the current context. Any draw calls after this
		 * method is called will use this shader to process, until another
		 * program is `use()`-ed.
		 */
		void use() const;

		/**
		 * Check if the program instance is valid. A program should be valid
		 * unless it has thrown an exception or been moved from. Calling any
		 * member functions on an invalid program is undefined behavior,
		 * although it is likely to raise an assertion.
		 */
		bool is_valid() const;

		/**
		 * Retrieve the OpenGL `name` (the instance identifier) for the program.
		 * Use with caution; the identifier is still managed by the instance,
		 * and changes made directly to the identifier may not be reflected.
		 * Note that an assertion is raised if `is_valid()` would equal `false`.
		 */
		GLuint get() const;

		/**
		 * Retrieve the specified attribute, if it exists. The attribute must be
		 * active and have been discovered through introspection.
		 */
		attribute get_attribute(const std::string&) const;

	private:
		// Link the specified `GL_SHADER` identifiers to this program.
		void link(const std::vector<GLuint>& names, const std::string& from);

	private:
		// The OpenGL identifier for this program.
		GLuint m_name;

		// The introspected attribute names mapped to their values.
		std::map<std::string, attribute> m_attributes;
	};

	/**
	 * See forward declaration in `program`.
	 */
	class program::shader_name {
	public:
		/**
		 * Create a new reference to the given shader.
		 */
		shader_name(const shader&);

	public:
		/**
		 * Retrieve the `name` of the shader this object references.
		 */
		GLuint get() const;

	private:
		// The OpenGL identifier, extracted from `shader::get()`.
		GLuint m_name;
	};
}
