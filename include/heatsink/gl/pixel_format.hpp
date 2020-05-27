#pragma once

#include <cstdlib>
#include <type_traits>

#include <heatsink/platform/gl.hpp>
#include <heatsink/traits/enum.hpp>
#include <heatsink/traits/tensor.hpp>

namespace heatsink::gl {
	/**
	 * An OpenGL pixel format, used to describe texture data on disk. This
	 * includes a format enumeration and a data, which are used to describe the
	 * data to load into a texture object.
	 */
	class pixel_format {
	public:
		/**
		 * Create a pixel format from the given tensor type. This deduces the
		 * number of channels (based on the extent) and the inner type (based
		 * on the no-extent base) from the template parameter, before passing
		 * the information to the normal `pixel_format()` constructor.
		 */
		template<tensor T>
		static constexpr pixel_format from_type(bool reverse = false);

	public:
		/**
		 * Create a pixel format from the given internal format. This may be the
		 * format a texture object was created with, or a simple format
		 * specifier. The data type chosen will be the closest match to the
		 * sized format, or `GL_UNSIGNED_BYTE` if unsized (see
		 * `underlying_enum()`). If the format is reversed, the data stored on
		 * disk is laid out with the blue channel first, i.e., `BGR` or `BGRA`.
		 */
		pixel_format(GLenum image_format, bool reverse = false);

	private:
		// Create a pixel format from the given base format and type. No
		// deduction occurs here; the values are used within the class as-is
		// (unless reverse is true).
		pixel_format(GLenum format, GLenum type, bool reverse = false);

	public:
		/**
		 * Retrieve the OpenGL format enumeration for this pixel format.
		 */
		GLenum get() const;
		/**
		 * Retrieve the OpenGL type enumeration for this pixel format.
		 */
		GLenum get_datatype() const;

		/**
		 * Retrieve the size, in bytes, a single pixel of this format consumes.
		 */
		std::size_t get_size() const;

	private:
		// The deduced image format.
		GLenum m_format;
		// The deduced image data type.
		GLenum m_datatype;
	};
}

namespace heatsink::gl {
	template<tensor T>
	constexpr pixel_format pixel_format::from_type(bool reverse) {
		using value_type = tensor_decay_t<T>;
		// Pixel formats are always one dimensional.
		static_assert(std::rank_v<value_type> <= 1);

		using component_type = std::remove_all_extents_t<value_type>;
		// The element type must have a `GLenum` analogue.
		constexpr auto component = make_enum_v<component_type>;
		static_assert(component != GL_NONE);

		GLenum format = GL_NONE;
		switch (component_count) {
			case 0: format = GL_RED;
			case 1: format = GL_RED;
			case 2: format = GL_RG;
			case 3: format = GL_RGB;
			case 4: format = GL_RGBA;
		}

		return pixel_format(format, component, reverse);
	}
}
