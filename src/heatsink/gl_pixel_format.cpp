#include <heatsink/gl/pixel_format.hpp>

#include <cassert>
#include <iostream>

#include <heatsink/error/exception.hpp>
#include <heatsink/traits/format.hpp>
#include <heatsink/traits/memory.hpp>

namespace heatsink::gl {
	pixel_format::pixel_format(GLenum image_format, bool reverse)
	: m_format{format_traits::remove_size(image_format)}, m_datatype{format_traits::underlying_datatype(image_format)} {
		if (reverse) {
			auto [f, t] = format_traits::reverse(image_format);
			if (f == GL_NONE) {
				std::cerr << "[heatsink::gl::pixel_format] image format '" << image_format << "' cannot be reversed." << std::endl;
				throw exception("gl::pixel_format", "image format is not reversible.");
			}

			m_format   = f;
			m_datatype = t;
		}
	}

	pixel_format::pixel_format(GLenum format, GLenum type, bool reverse)
	: m_format{format_traits::remove_size(format)}, m_datatype{type} {
		if (reverse) {
			// Note that the reversed data type is not used in this constructor;
			// only the format itself may be reversed when specified separately.
			auto [f, t] = format_traits::reverse(format);
			if (f == GL_NONE) {
				std::cerr << "[heatisnk::gl::pixel_format] format '" << format << "' cannot be reversed." << std::endl;
				throw exception("gl::pixel_format", "format is not reversible.");
			}

			m_format = f;
		}
	}

	GLenum pixel_format::get() const {
		return m_format;
	}

	GLenum pixel_format::get_datatype() const {
		return m_datatype;
	}

	std::size_t pixel_format::get_size() const {
		auto datasize = size_of(m_datatype);
		assert(datasize != 0);

		return (is_packed(m_datatype)) ? datasize : datasize * format_traits::extent(m_format);
	}
}
