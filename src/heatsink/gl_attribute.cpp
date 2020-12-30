#include <heatsink/gl/attribute.hpp>

#include <cassert>
#include <iostream>
#include <numeric>
#include <tuple>
#include <vector>

#include <heatsink/error/exception.hpp>
#include <heatsink/gl/program.hpp>
#include <heatsink/traits/shader.hpp>

namespace heatsink::gl {
	std::map<std::string, attribute> attribute::from_program(const program& p) {
		std::map<std::string, attribute> results;
		GLint count;

		glGetProgramiv(p.get(), GL_ACTIVE_ATTRIBUTES, &count);
		std::vector<GLuint> indices(count);

		// Pre-allocate storage for the names since there is no method to easily
		// get the exact size per-attribute beforehand.
		glGetProgramiv(p.get(), GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &count);
		std::vector<char> buffer(count);

		// Create a list 0,1,2,3... for all active attributes.
		std::iota(indices.begin(), indices.end(), 0);
		for (const auto i : indices) {
			auto [type, size, namesize] = std::tuple<GLenum, GLint, GLsizei>{};
			glGetActiveAttrib(p.get(), i, (GLsizei)std::ssize(buffer), &namesize, &size, &type, buffer.data());
			
			std::string name(buffer.data(), namesize);
			if (!shader_traits::is_builtin(name))
				results.emplace(name, attribute(p, name, type, size));
		}

		return results;
	}

	// A datatype of `GL_NONE` is used to determine whether this attribute has
	// been constructed through an annotation.
	attribute::attribute(const program& p, std::string name)
	: attribute(p, std::move(name), GL_NONE, 0) {}

	attribute::attribute(std::size_t location)
	: m_location(m_location), m_datatype{GL_NONE} {}

	attribute::attribute(const program& p, std::string name, GLenum type, GLsizei size)
	: m_name{std::move(name)}, m_datatype{type}, m_size(size) {
		m_location = glGetAttribLocation(p.get(), m_name.c_str());
		if (m_location == -1) {
			std::cerr << "[heatsink::gl::attribute] unknown attribute name '" << m_name << "'." << std::endl;
			throw exception("gl::attribute", "could not find attribute location.");
		}
	}

	bool attribute::is_valid() const {
		return (m_location != -1);
	}

	bool attribute::is_annotated() const {
		assert(this->is_valid());
		return (m_datatype != GL_NONE);
	}

	GLuint attribute::get() const {
		assert(this->is_valid());
		return (GLuint)m_location;
	}

	const std::string& attribute::get_name() const {
		assert(this->is_valid() && this->is_annotated());
		return m_name;
	}

	GLenum attribute::get_datatype() const {
		assert(this->is_valid() && this->is_annotated());
		return m_datatype;
	}

	std::size_t attribute::get_size() const {
		assert(this->is_valid() && this->is_annotated());
		return m_size;
	}
}

namespace std {
	using attribute = heatsink::gl::attribute;

	bool less<attribute>::operator ()(const attribute& lhs, const attribute& rhs) const {
		return (lhs.get() < rhs.get());
	}
}
