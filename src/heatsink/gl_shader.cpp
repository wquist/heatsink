#include <heatsink/gl/shader.hpp>

#include <cassert>
#include <iostream>
#include <fstream>

#include <heatsink/error/compile.hpp>
#include <heatsink/error/exception.hpp>

namespace {
	using exception = heatsink::exception;

	// Read the contents of the specified file.
	std::string read_string(const std::filesystem::path& path) {
		std::ifstream data(path.native());
		if (!data.is_open())
			throw exception("gl::shader", "could not open path.");

		std::istreambuf_iterator<char> begin{data};
		return std::string(begin, {});
	}

	GLenum to_stage(const std::filesystem::path& path) {
		auto ext = path.extension();
		// Find the extension before the ".glsl"
		if (ext == ".glsl")
			ext = path.stem().extension();

		if      (ext == ".vert") return GL_VERTEX_SHADER;
		else if (ext == ".frag") return GL_FRAGMENT_SHADER;
		else if (ext == ".tesc") return GL_TESS_CONTROL_SHADER;
		else if (ext == ".tese") return GL_TESS_EVALUATION_SHADER;
		else if (ext == ".geom") return GL_GEOMETRY_SHADER;
		else if (ext == ".comp") return GL_COMPUTE_SHADER;
		else
			throw exception("gl::shader", "unknown GLSL extension.");
	}
}

namespace heatsink::gl {
	shader shader::from_file(const std::filesystem::path& path, GLenum stage) {
		std::ifstream data(path.native());
		if (!data.is_open())
			throw heatsink::exception("gl::shader", "could not open path.");

		std::istreambuf_iterator<char> begin{data};
		if (stage == GL_NONE)
			stage = to_stage(path);

		return shader(std::string(begin, {}), stage, path.filename().string());
	}

	shader::shader(const std::string& src, GLenum stage, const std::string& from)
	: m_name{glCreateShader(stage)}, m_stage{stage} {
		if (!m_name)
			throw exception("gl::shader", "could not create shader object.");

		this->compile(src, from);
	}

	shader::shader(shader&& other) noexcept
	: m_name{other.m_name}, m_stage{other.m_stage} {
		other.m_name = 0;
	}

	shader::~shader() {
		if (m_name)
			glDeleteShader(m_name);
	}

	shader& shader::operator =(shader&& other) noexcept {
		if (m_name)
			glDeleteShader(m_name);

		m_name  = other.m_name;
		m_stage = other.m_stage;

		other.m_name = 0;
		return *this;
	}

	bool shader::is_valid() const {
		return (m_name != 0);
	}

	GLuint shader::get() const {
		assert(this->is_valid());
		return m_name;
	}

	GLenum shader::get_stage() const {
		assert(this->is_valid());
		return m_stage;
	}

	void shader::compile(const std::string& src, const std::string& from) {
		const auto* raw = src.c_str();
		glShaderSource(m_name, 1, &raw, nullptr);

		glCompileShader(m_name);

		GLint result;
		if (glGetShaderiv(m_name, GL_COMPILE_STATUS, &result); result != GL_TRUE) {
			std::cerr << "[heatsink::gl::shader] shader compilation errors:" << std::endl;
			write_shader_log(std::cerr, m_name, from);

			throw exception("gl::shader", "could not compile shader source.");
		}
	}
}
