#pragma once

#include <cstdlib>
#include <map>
#include <string>

#include <heatsink/platform/gl.hpp>

namespace heatsink::gl {
	/**
	 * An OpenGL vertex attribute. These are defined in shader programs that
	 * include a vertex shader, and are used to set up the attribute arrays of
	 * a vertex array object (VAO). This class functions in two "modes":
	 * annotated and non-annotated. Attribute information can be retrieved from
	 * a shader program (annotated) where type, size, and name info is also
	 * available. Attributes can also be referenced solely by their location
	 * (non-annotated), where no additional information is available.
	 */
	class attribute {
	public:
		/**
		 * Retrieve information on all active attributes in a shader program.
		 * This creates a map of annotation names to their location and other
		 * information. Usually, a shader program will call this function and
		 * add the results to its internally tracked attributes.
		 */
		static std::map<std::string, attribute> from_program(const class program&);

	public:
		/**
		 * Create a non-annotated attribute, determining its location from its
		 * name in the given shader program. Note that the name is not stored,
		 * since it should not be accessible in a non-annotated attribute.
		 */
		attribute(const class program&, std::string name);
		/**
		 * Create a non-annotated attribute with the given location.
		 */
		attribute(std::size_t location);

	private:
		// Create the attribute with all information specified. This constructor
		// is used by the public variants and by introspection.
		attribute(const class program&, std::string name, GLenum type, GLsizei size);

	public:
		/**
		 * Check if this attribute is valid, which should be the case unless it
		 * failed during construction (such as using a non-existant attribute
		 * name). Using an invalid attribute is undefined and will usually raise
		 * an assertion.
		 */
		bool is_valid() const;
		/**
		 * Check if this attribute is "annotated" (created through
		 * introspection). If the attribute is not annotated, attempting to
		 * retrieve the name, size, or type information will raise an assertion.
		 */
		bool is_annotated() const;

		/**
		 * Retrieve the attribute location of this instance. This is the index
		 * value used in `glVertexAttrib` and friends.
		 */
		GLuint get() const;
		/**
		 * Retrieve the name of this attribute, if annotated.
		 */
		const std::string& get_name() const;

		/**
		 * Retrieve the OpenGL enumeration value (`GL_FLOAT_VEC2`, etc.) of this
		 * attribute, if annotated.
		 */
		GLenum get_datatype() const;
		/**
		 * Retrieve the number of attribute indices needed to represent this
		 * (array) type, if annotated.
		 */
		std::size_t get_size() const;

	private:
		// The attribute location (always castable to `GLuint` when valid).
		GLint m_location;
		// The annotated name.
		std::string m_name;

		// The annotated GL enumeration type.
		GLenum m_datatype;
		// The array size of the annotated type.
		std::size_t m_size;
	};
}

namespace std {
	// Implement a comparator for `attribute` so it can be used in the sorted
	// standard library containers. An `attribute` is sorted on its location.
	template<>
	struct less<heatsink::gl::attribute> {
		bool operator ()(const heatsink::gl::attribute&, const heatsink::gl::attribute&) const;
	};
}
