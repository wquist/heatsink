#include <heatsink/gl/vertex_format.hpp>

#include <cassert>
#include <iostream>

#include <heatsink/error/exception.hpp>
#include <heatsink/traits/memory.hpp>

namespace heatsink::gl {
	vertex_format::vertex_format(GLenum e)
	: vertex_format(remove_all_extents(e), {extent(e, 0), extent(e, 1)}) {}

	vertex_format::vertex_format(GLenum datatype, extents es, packing pack)
	: m_datatype{datatype}, m_extents{es}, m_packing{pack} {
		if (m_extents[0] == 0 || m_extents[1] == 0)
			throw exception("gl::vertex_format", "format extents cannot be zero.");
		if (m_extents[0] > 4)
			throw exception("gl::vertex_format", "format cannot specify more than 4 components.");

		// Calculate the format stride before modifying index/component values.
		auto format_size = size_of(m_datatype) * m_extents[0] * m_extents[1];
		// The datatype should always be a valid GL type.
		assert(format_size > 0);

		if (m_packing.stride == 0)
			m_packing.stride = format_size;
		if (format_size > m_packing.stride) {
			std::cerr << "calculated format size (size=" << format_size << ") is greater than specified stride";
			std::cerr << " (stride=" << m_packing.stride << ")." << std::endl;

			throw exception("gl::vertex_format", "invalid stride specified for format.");
		}

		// If the format uses a double type, each component takes up twice as
		// much space. Because an attribute slot can only hold four float
		// values, only two double components can be represented per index.
		// FIXME: should this be here, or in vertex_array?
		if (m_datatype == GL_DOUBLE)
			m_extents[1] *= 2;
	}

	GLenum vertex_format::get_datatype() const {
		return m_datatype;
	}

	vertex_format::extents vertex_format::get_extents() const {
		return m_extents;
	}

	vertex_format::packing vertex_format::get_packing() const {
		return m_packing;
	}
}
