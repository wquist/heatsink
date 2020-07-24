#pragma once

#include <heatsink/gl/attribute.hpp>
#include <heatsink/gl/vertex_format.hpp>
#include <heatsink/platform/gl.hpp>

namespace heatsink::gl {
	/**
	 * An OpenGL vertex array object (VAO). This holds vertex format and buffer
	 * state for a set of attributes. Note that the class does not store this
	 * information; it is maintained in the OpenGL state machine.
	 */
	class vertex_array : public object<GL_VERTEX_ARRAY> {
	public:
		/**
		 * Specifies the conversion type when binding buffer data to a vertex
		 * array object. Normally, any buffer data is converted to a normalized
		 * float. However, the type can be specified (using
		 * `glVertexAttribIPointer`/`glVertexAttribLPointer`) by specifying one
		 * of the conversions below. The `floating_point` value represents an
		 * un-normalized floating point conversion.
		 */
		enum class conversion {
			integer,
			floating_point,
			double_precision
		};

	public:
		/**
		 * Create a vertex array object. Note that no parameters are needed as
		 * a VAO relies on no initial state.
		 */
		vertex_array();

	public:
		/**
		 * Set the format and buffer data for the specified attribute. The
		 * attribute may be obtained from a shader or created directly by its
		 * location. Note that any annotations that may exist will not be used.
		 * The buffer data is always normalized to a floating point range.
		 */
		void set_attribute(const attribute&, vertex_format, buffer::const_view);
		/**
		 * Set the information for an attribute, specifying the conversion type.
		 * This allows for buffer data to be directly interpreted as various
		 * primitive types. See the above overload.
		 */
		void set_attribute(const attribute&, vertex_format, buffer::const_view, conversion);
		/**
		 * Reset a given attribute (and disable it). The given attribute will no
		 * longer be used when the vertex array is bound. Note that if the given
		 * attribute is not set, this call is effectively ignored.
		 */
		void set_attribute(const attribute&, std::nullptr_t);

		/**
		 * Set the `GL_ELEMENT_ARRAY_BUFFER` bind point to the given index
		 * buffer. The buffer must already have been bound to this target.
		 */
		void set_elements(const buffer&);
		/**
		 * Unset/reset the index buffer bind point for this vertex array.
		 */
		void set_elements(std::nullptr_t);

	private:
		// Combined attribute setter for the normalized and conversion variants.
		void set_attribute(const attribute&, vertex_format, buffer::const_view, conversion*);
	};
}