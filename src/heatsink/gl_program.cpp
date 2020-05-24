#include <heatsink/gl/program.hpp>

#include <cassert>
#include <iostream>

#include <heatsink/error/compile.hpp>
#include <heatsink/error/exception.hpp>

namespace {
	using shader_name = heatsink::gl::program::shader_name;

	// Convert a set of `shader`/`shader_name`s into their `GLuint` values.
	template<class T>
	std::vector<GLuint> to_names(const std::vector<T>& shaders) {
		std::vector<GLuint> names;
		for (const auto& s : shaders)
			names.push_back(s.get());

		return names;
	}
}

namespace heatsink::gl {
	program program::from_files(const std::vector<std::filesystem::path>& paths) {
		// The vector `shaders` must exist until the `program` is constructed,
		// or the OpenGL names will be freed too early.
		std::vector<shader> shaders;
		for (const auto& p : paths)
			shaders.push_back(shader::from_file(p));

		// By default, use the first specified shader as the ID.
		auto from = paths.front();
		// Remove all extensions so that only the base name is present. Most
		// programs will probably use the same base name for each stage, so this
		// should be easily identifiable in the logs.
		while (!from.extension().empty())
			from = from.stem();

		return program(to_names(shaders), from.string());
	}

	program::program(const std::vector<shader_name>& shaders, const std::string& from)
	: program(to_names(shaders), from) {}

	program::program(program&& other) noexcept
	: m_name{other.m_name} {
		other.m_name = 0;
	}

	program::~program() {
		if (m_name)
			glDeleteProgram(m_name);
	}

	program& program::operator =(program&& other) noexcept {
		if (m_name)
			glDeleteProgram(m_name);

		m_name = other.m_name;

		other.m_name = 0;
		return *this;
	}

	program::program(const std::vector<GLuint>& names, const std::string& from)
	: m_name{glCreateProgram()} {
		this->link(names, from);
	}

	void program::use() const {
		assert(this->is_valid());
		glUseProgram(m_name);
	}

	bool program::is_valid() const {
		return (m_name != 0);
	}

	GLuint program::get() const {
		assert(this->is_valid());
		return m_name;
	}

	void program::link(const std::vector<GLuint>& names, const std::string& from) {
		for (const auto& n : names)
			glAttachShader(m_name, n);

		glLinkProgram(m_name);

		GLint result;
		if (glGetProgramiv(m_name, GL_LINK_STATUS, &result); result != GL_TRUE) {
			std::cerr << "[heatsink::gl::program] program link errors:" << std::endl;
			write_program_log(std::cerr, m_name, from);

			throw exception("gl::program", "could not link shader sources.");
		}

		// Once the program is set up, the shaders no longer need to be
		// associated with the `GL_PROGRAM` object.
		for (const auto& n : names)
			glDetachShader(m_name, n);
	}

	program::shader_name::shader_name(const shader& s)
	: m_name{s.get()} {}

	GLuint program::shader_name::get() const {
		return m_name;
	}
}
