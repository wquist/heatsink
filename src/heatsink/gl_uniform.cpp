#include <heatsink/gl/uniform.hpp>

#include <numeric>
#include <vector>

#include <heatsink/gl/program.hpp>

namespace {
	std::vector<GLint> get_parameters(GLuint p, const std::vector<GLuint>& indices, GLenum e) {
		std::vector<GLint> results(indices.size());
		glGetActiveUniformsiv(p, (GLsizei)indices.size(), indices.data(), e, results.data());

		return results;
	}

	void validate_location(GLint location, const std::string& name) {
		if (location == -1) {
			std::cerr << "[heatsink::gl::uniform] unknown uniform name '" << name << "'." << std::endl;
			throw heatsink::exception("gl::uniform", "could not find uniform location.");
		}
	}
}

namespace heatsink::gl {
	std::map<std::string, uniform> uniform::from_program(const program& p) {
		auto owner = p.get();
		std::map<std::string, uniform> results;

		GLint count;
		glGetProgramiv(owner, GL_ACTIVE_UNIFORMS, &count);
		std::vector<GLuint> indices(count);

		// Collect all info using the `glGetActiveUniformsiv` to prevent from
		// calling OpenGL 3 or 4 times for each uniform.
		auto types   = get_parameters(owner, indices, GL_UNIFORM_TYPE);
		auto sizes   = get_parameters(owner, indices, GL_UNIFORM_SIZE);
		auto blocks  = get_parameters(owner, indices, GL_UNIFORM_BLOCK_INDEX);
		auto lengths = get_parameters(owner, indices, GL_UNIFORM_NAME_LENGTH);

		// The indices are guaranteed to be in the range [0, ACTIVE_UNIFORMS).
		std::iota(indices.begin(), indices.end(), 0);
		for (const auto i : indices) {
			// If the uniform is part of a block it must be handled differently.
			if (blocks[i] != -1)
				continue;

			auto u = uniform(owner, i, types[i], sizes[i], lengths[i]);
			results.emplace(u.get_name(), u);
		}

		return results;
	}

	uniform::uniform(const program& p, std::string name)
	: m_program{p.get()}, m_name{std::move(name)}, m_base{} {
		const auto* cname = m_name.c_str();
		std::vector<GLuint> index(1);

		m_location = glGetUniformLocation(m_program, cname);
		validate_location(m_location, m_name);

		glGetUniformIndices(m_program, 1, &cname, index.data());
		// The index should never be invalid at this point.
		assert(index[0] != GL_INVALID_INDEX);

		m_datatype = get_parameters(m_program, index, GL_UNIFORM_TYPE)[0];
		m_size     = get_parameters(m_program, index, GL_UNIFORM_SIZE)[0];
		// Ensure this uniform is not in a block (which is handled separately).
		if (get_parameters(m_program, index, GL_UNIFORM_BLOCK_INDEX)[0] != -1) {
			std::cerr << "[heatsink::gl::uniform] uniform '" << m_name << "' is part of a uniform block." << std::endl;
			throw exception("gl::uniform", "uniform cannot be part of a block.");
		}
	}

	uniform::uniform(GLuint owner, GLuint index, GLuint type, std::size_t size, std::size_t namelen)
	: m_program{owner}, m_datatype{type}, m_size{size} {
		// `std::string::resize` does not include the null terminator.
		m_name.resize(namelen - 1);
		// The number of chars to write is known, so the result is ignored.
		glGetActiveUniformName(m_program, index, (GLsizei)namelen, nullptr, m_name.data());

		// Array uniforms will have an introspected name with the subscript
		// appended (even the first element will have "[0]"), so remove this to
		// get the name as if it was passed in the public constructor.
		if (auto bracket = m_name.find('['); bracket != std::string::npos)
			m_name.resize(bracket);

		m_location = glGetUniformLocation(m_program, m_name.c_str());
		validate_location(m_location, m_name);
	}

	uniform::uniform(const uniform& u, std::size_t offset, std::size_t size)
	: m_program{u.m_program}, m_name{u.m_name}, m_datatype{u.m_datatype}, m_base{u.m_base + offset}, m_size{size} {
		// The name is not exposed in uniform view objects, but the new name
		// must still be calculated to get the new location.
		auto name = m_name + "[" + std::to_string(m_base) + "]";

		m_location = glGetUniformLocation(m_program, name.c_str());
		validate_location(m_location, name);
	}

	uniform::view uniform::make_view(std::size_t offset, std::size_t size) {
		assert(this->is_valid());
		if (!this->is_array())
			throw exception("gl::uniform", "cannot make view of non-array uniform.");

		if (offset + size > m_size) {
			std::cerr << "[heatsink::gl::uniform] could not make view (offset=" << offset << ", size=" << size;
			std::cerr << ") out of uniform '" << m_name << "' (size=" << m_size << ")." << std::endl;

			throw exception("gl::uniform", "uniform view range out of bounds.");
		}

		return view(*this, offset, size);
	}

	bool uniform::is_valid() const {
		return (m_location != -1);
	}

	bool uniform::is_array() const {
		assert(this->is_valid());
		return (m_size > 1);
	}

	GLuint uniform::get() const {
		assert(this->is_valid());
		return m_location;
	}

	const std::string& uniform::get_name() const {
		assert(this->is_valid());
		return m_name;
	}

	GLenum uniform::get_datatype() const {
		assert(this->is_valid());
		return m_datatype;
	}

	std::size_t uniform::get_size() const {
		assert(this->is_valid());
		return m_size;
	}

	std::size_t uniform::get_base() const {
		return m_base;
	}

	void uniform::update_values(GLenum type, std::size_t count, const bool* bs) {
		// Booleans can be used directly (to distinguish between `GLbool` and
		// `GLchar`, etc.), but OpenGL shader functions require them to be
		// loaded as `GLint`; therefore, they are copied into a temp array.
		std::vector<GLint> is(count);
		std::copy(bs, bs + count, is.begin());

		glUseProgram(m_program);
		switch (type) {
			case GL_BOOL:      glUniform1iv(m_location, (GLsizei)count, is.data()); break;
			case GL_BOOL_VEC2: glUniform2iv(m_location, (GLsizei)count, is.data()); break;
			case GL_BOOL_VEC3: glUniform3iv(m_location, (GLsizei)count, is.data()); break;
			case GL_BOOL_VEC4: glUniform4iv(m_location, (GLsizei)count, is.data()); break;
		}
	}

	void uniform::update_values(GLenum type, std::size_t count, const GLint* is) {
		glUseProgram(m_program);
		switch (type) {
			case GL_INT:      glUniform1iv(m_location, (GLsizei)count, is); break;
			case GL_INT_VEC2: glUniform2iv(m_location, (GLsizei)count, is); break;
			case GL_INT_VEC3: glUniform3iv(m_location, (GLsizei)count, is); break;
			case GL_INT_VEC4: glUniform4iv(m_location, (GLsizei)count, is); break;
		}
	}

	void uniform::update_values(GLenum type, std::size_t count, const GLuint* us) {
		glUseProgram(m_program);
		switch (type) {
			case GL_UNSIGNED_INT:      glUniform1uiv(m_location, (GLsizei)count, us); break;
			case GL_UNSIGNED_INT_VEC2: glUniform2uiv(m_location, (GLsizei)count, us); break;
			case GL_UNSIGNED_INT_VEC3: glUniform3uiv(m_location, (GLsizei)count, us); break;
			case GL_UNSIGNED_INT_VEC4: glUniform4uiv(m_location, (GLsizei)count, us); break;
		}
	}

	void uniform::update_values(GLenum type, std::size_t count, const GLfloat* fs) {
		glUseProgram(m_program);
		switch (type) {
			case GL_FLOAT:      glUniform1fv(m_location, (GLsizei)count, fs); break;
			case GL_FLOAT_VEC2: glUniform2fv(m_location, (GLsizei)count, fs); break;
			case GL_FLOAT_VEC3: glUniform3fv(m_location, (GLsizei)count, fs); break;
			case GL_FLOAT_VEC4: glUniform4fv(m_location, (GLsizei)count, fs); break;

			case GL_FLOAT_MAT2:   glUniformMatrix2fv(  m_location, (GLsizei)count, GL_FALSE, fs); break;
			case GL_FLOAT_MAT2x3: glUniformMatrix2x3fv(m_location, (GLsizei)count, GL_FALSE, fs); break;
			case GL_FLOAT_MAT2x4: glUniformMatrix2x4fv(m_location, (GLsizei)count, GL_FALSE, fs); break;

			case GL_FLOAT_MAT3x2: glUniformMatrix3x2fv(m_location, (GLsizei)count, GL_FALSE, fs); break;
			case GL_FLOAT_MAT3:   glUniformMatrix3fv(  m_location, (GLsizei)count, GL_FALSE, fs); break;
			case GL_FLOAT_MAT3x4: glUniformMatrix3x4fv(m_location, (GLsizei)count, GL_FALSE, fs); break;

			case GL_FLOAT_MAT4x2: glUniformMatrix4x2fv(m_location, (GLsizei)count, GL_FALSE, fs); break;
			case GL_FLOAT_MAT4x3: glUniformMatrix4x3fv(m_location, (GLsizei)count, GL_FALSE, fs); break;
			case GL_FLOAT_MAT4:   glUniformMatrix4fv(  m_location, (GLsizei)count, GL_FALSE, fs); break;
		}
	}

	uniform::view uniform::operator [](std::size_t index) {
		return this->make_view(index, 1);
	}

	uniform::view::view(uniform& u, std::size_t offset, std::size_t size)
	: uniform(u, offset, size) {}

	uniform::view::view(view& v, std::size_t offset, std::size_t size)
	: uniform(v, offset, size) {}

	std::size_t uniform::view::get_offset() const {
		assert(this->is_valid());
		return uniform::get_base();
	}
}
