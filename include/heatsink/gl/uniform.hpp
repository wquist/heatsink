#pragma once

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <type_traits>

#include <heatsink/error/exception.hpp>
#include <heatsink/platform/gl.hpp>
#include <heatsink/traits/enum.hpp>
#include <heatsink/traits/memory.hpp>
#include <heatsink/traits/shader.hpp>
#include <heatsink/traits/tensor.hpp>

namespace heatsink::gl {
	/**
	 * A non-block OpenGL active uniform. This class must be constructed with
	 * reference to an already-existing `program`; its lifetime is also tied to
	 * the used program; destroying the program and using one of its uniforms
	 * will result in undefined behavior.
	 */
	class uniform {
	public:
		/**
		 * A subset of an array uniform. Allows a portion of the parent uniform
		 * to be set without setting/re-setting the other values. Note that the
		 * lifetime of a view is independent of its uniform; as long as the
		 * source `program` exists, it may be used. This allows functions to
		 * return a `uniform` view directly and discard the "full-view" uniform.
		 */
		class view;

	public:
		/**
		 * Retrieve information on all active uniforms in a shader program. This
		 * creates a map of annotation names to their location and other
		 * information. Usually, a shader program will call this function and
		 * add the results to its internally tracked uniforms.
		 */
		static std::map<std::string, uniform> from_program(const class program&);

	public:
		/**
		 * Construct a uniform from a program and the its instance name.
		 */
		uniform(const class program&, std::string name);

	private:
		// Create a uniform with all parameters specified. This constructor is
		// used by the public variants and introspection.
		uniform(GLuint owner, GLuint type, std::size_t size, std::size_t namelen);
		// Copy a uniform, but alter the parameters appropriate for a `view`
		// within the given range. Used by the child `view` constructor.
		uniform(const uniform&, std::size_t offset, std::size_t size);

	public:
		// Set the uniform to a single value. The size of the data must match
		// the uniform; that is, the uniform must not be an array.
		template<tensor T> requires (std::is_array_v<T> == false)
		void update(const T&);
		// Update the values of an array uniform. Note that, like above, the
		// size of the given array must match the size of the uniform.
		template<std::contiguous_iterator Iterator>
		void update(Iterator begin, Iterator end);

		/**
		 * Create a new view out of a subset of this uniform. Note that the
		 * uniform must be an array type.
		 */
		view make_view(std::size_t offset, std::size_t size);

		/**
		 * Check if the uniform instance is valid. A uniform should be valid
		 * unless it has thrown an exception (probably during construction).
		 * Calling any member functions on an invalid uniform is undefined
		 * and will usually raise an assertion.
		 */
		bool is_valid() const;
		/**
		 * Check if the uniform is an array type.
		 */
		bool is_array() const;

		/**
		 * Retrieve the location of this uniform within its program object.
		 */
		GLuint get() const;
		/**
		 * Retrieve the name of this uniform. If the uniform is an array, it
		 * will be introspected with a `[0]` prepended to it; this implicit zero
		 * index is not stored in the name, as the index-less variants
		 * represents the same value.
		 */
		const std::string& get_name() const;

		/**
		 * Retrieve the OpenGL type enumeration of this uniform. This contains
		 * both the data type and the number of elements that make up the value
		 * as a whole (such as `GL_FLOAT_VEC4` for a vec4).
		 */
		GLenum get_datatype() const;
		/**
		 * Retrieve the "size" of this uniform; its array length. Calling this
		 * on a non-array uniform will raise an assertion.
		 */
		std::size_t get_size() const;

	protected:
		// Allow subclass access to the base "offset" managed by this uniform.
		std::size_t get_offset() const;

	private:
		// Overloads to pass uniform data to the correct `glUniform*()` variant.
		void update(GLenum type, std::size_t count, const bool*);
		void update(GLenum type, std::size_t count, const GLint*);
		void update(GLenum type, std::size_t count, const GLuint*);
		void update(GLenum type, std::size_t count, const GLfloat*);

	public:
		/**
		 * Allow the program uniform value to be set directly. Note that, like
		 * `update()`, this method is only valid for non-array uniforms.
		 */
		template<tensor T> requires (std::is_array_v<T> == false)
		uniform& operator =(const T&);
		/**
		 * Allow single-element views to be quickly created (for easy setting).
		 * This operator is equivalent to `make_view(index, 1)`. Note that the
		 * uniform must be an array type or an assertion will be raised.
		 */
		view operator [](std::size_t index);

	private:
		// The OpenGL identifier of the owning program.
		GLuint m_program;
		// The index of this uniform within the owning program (note that this
		// is not necessarily the same as the location, especially when dealing
		// with array types.
		GLuint m_index;

		// The location of this uniform within the owning program (always
		// castable to a `GLuint` when valid).
		GLint m_location;
		// The string identifier of this uniform.
		std::string m_name;

		// The GL enumeration type of this uniform.
		GLenum m_datatype;
		// The index within an array uniform. Used by subclasses.
		std::size_t m_base;
		// The array size of this uniform (if an array).
		std::size_t m_size;
	};

	/**
	 * See forward declaration in `uniform`.
	 */
	class uniform::view : private uniform {
	public:
		/**
		 * Construct a new view from the given uniform, with the specified
		 * range. The uniform must be an array with more than one element.
		 */
		view(uniform&, std::size_t offset, std::size_t size);
		/**
		 * Construct a new view from the given existing view, with the specified
		 * range. See the above overload.
		 */
		view(view&, std::size_t offset, std::size_t size);

	public:
		/**
		 * Retrieve the index of this view into the parent array uniform.
		 */
		std::size_t get_offset() const;

		/**
		 * A view implements a subset of the `uniform` interface. All methods
		 * function exactly the same as they would on the uniform, only on the
		 * range specified.
		 */
		using uniform::update;
		using uniform::make_view;

		using uniform::is_valid;
		using uniform::is_array;
		
		using uniform::get;
		using uniform::get_size;

	public:
		using uniform::operator =;
		using uniform::operator [];
	};
}

namespace heatsink::gl {
	template<tensor T> requires (std::is_array_v<T> == false)
	void uniform::update(const T& t) {
		assert(this->is_valid() && !this->is_array());

		constexpr auto datatype = make_enum_v<tensor_decay_t<T>>;
		static_assert(datatype != GL_NONE);

		if (!shader_traits::is_assignable(m_datatype, datatype)) {
			std::cerr << "[heatsink::gl::uniform] cannot assign '" << datatype;
			std::cerr << "' to '" << m_datatype << "'." << std::endl;

			throw exception("gl::uniform", "type mismatch.");
		}

		this->update(datatype, 1, address_of(t));
	}

	template<std::contiguous_iterator Iterator>
	void uniform::update(Iterator begin, Iterator end) {
		assert(this->is_valid() && this->is_array());

		using T = typename std::iterator_traits<Iterator>::value_type;
		static_assert(is_tensor_v<T>);

		constexpr auto datatype = make_enum_v<tensor_decay_t<T>>;
		static_assert(datatype != GL_NONE);

		if (auto size = std::distance(begin, end); size != m_size) {
			std::cerr << "[heatsink::gl::uniform] cannot assign array of " << size;
			std::cerr << " to uniform (view) with " << m_size << " elements." << std::endl;

			throw exception("gl::uniform", "array size mismatch.");
		}
		if (!shader_traits::is_assignable(m_datatype, datatype)) {
			std::cerr << "[heatsink::gl::uniform] cannot assign '" << datatype;
			std::cerr << "' element to '" << m_datatype << "' array." << std::endl;

			throw exception("gl::uniform", "type mismatch.");
		}

		this->update(datatype, m_size, address_of(*begin));
	}

	template<tensor T> requires (std::is_array_v<T> == false)
	uniform& uniform::operator =(const T& t) {
		this->update(t);
		return *this;
	}
}
