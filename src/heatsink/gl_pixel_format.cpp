#include <heatsink/gl/pixel_format.hpp>

#include <heatsink/error/exception.hpp>
#include <heatsink/traits/format.hpp>

namespace heatsink::gl {
	pixel_format::pixel_format(GLenum format, GLenum type, bool reverse)
	: m_format{format_traits::remove_size(format)}, m_datatype{type} {
		if (reverse) {
			// Note that the reversed data type is not used in this constructor;
			// only the format itself may be reversed when specified separately.
			auto [f, t] = format_traits::reverse(format);
			if (f == GL_NONE)
				throw exception("gl::pixel_format", "format is not reversible.");

			m_format = f;
		}
	}

	pixel_format::pixel_format(GLenum image_format, bool reverse)
	: m_format{format_traits::remove_size(image_format)}, m_datatype{format_traits::underlying_datatype(image_format)} {
		if (reverse) {
			auto [f, t] = format_traits::reverse(image_format);
			if (f == GL_NONE)
				throw exception("gl::pixel_format", "image format is not reversible.");

			m_format   = f;
			m_datatype = t;
		}
	}

	GLenum pixel_format::get() const {
		return m_format;
	}

	GLenum pixel_format::get_datatype() const {
		return m_datatype;
	}
}
