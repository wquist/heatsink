#include <heatsink/gl/pixel_format.hpp>

#include <cassert>
#include <ostream>

#include <heatsink/error/debug.hpp>
#include <heatsink/error/exception.hpp>
#include <heatsink/traits/format.hpp>
#include <heatsink/traits/memory.hpp>

namespace {
	// A format passed to "format_traits::reverse()" may not always be reversible.
	void validate_reverse(GLenum result) {
		if (result == GL_NONE) {
			heatsink::make_error_stream("gl::pixel_format")
				<< "image format "
				<< heatsink::gl::to_string(result) << " "
				<< "is not reversible." << std::endl;
			
			throw heatsink::exception("gl::pixel_format", "could not reverse image format.");
		}
	}
}

namespace heatsink::gl {
	pixel_format::pixel_format(GLenum image_format, bool reverse) {
		if (reverse) {
			auto [f, t] = format_traits::reverse(image_format);
			validate_reverse(f);

			m_format   = f;
			m_datatype = t;
		} else {
			m_format = format_traits::remove_size(image_format);
			m_datatype = format_traits::underlying_datatype(image_format);
		}
	}

	pixel_format::pixel_format(GLenum format, GLenum type, bool reverse)
	: m_format{format_traits::remove_size(format)}, m_datatype{type} {
		if (reverse) {
			// Note that the reversed data type is not used in this constructor;
			// only the format itself may be reversed when specified separately.
			auto [f, t] = format_traits::reverse(format);
			validate_reverse(f);

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
		// Assert here since m_datatype is assumed to be a valid GL type.
		assert(datasize != 0);

		return (is_packed(m_datatype)) ? datasize : datasize * format_traits::extent(m_format);
	}
}
