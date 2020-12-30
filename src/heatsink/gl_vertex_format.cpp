#include <heatsink/gl/vertex_format.hpp>

#include <cassert>

#include <heatsink/traits/memory.hpp>

namespace heatsink::gl {
	vertex_format::vertex_format(GLenum e)
	: vertex_format(remove_all_extents(e), {extent(e, 0), extent(e, 1)}) {}

	vertex_format::vertex_format(GLenum datatype, extents es, packing pack)
	: m_datatype{datatype}, m_packing{pack} {
		// Allow extents to be specified shorthand (i.e. `1` when there is no
		// array index), since a zero component/index has no meaning.
		m_extents = glm::max(es, glm::uvec2(1));

		// Calculate the format stride before modifying index/component values.
		auto format_size = size_of(m_datatype) * m_extents[0] * m_extents[1];
		assert(format_size > 0);

		if (m_packing.stride == 0)
			m_packing.stride = format_size;
		// If the format uses a double type, each component takes up twice as
		// much space. Because an attribute slot can only hold four float
		// values, only two double components can be represented per index.
		if (m_datatype == GL_DOUBLE)
			m_extents[1] *= 2;

		// Check some basic constraints to ensure the passed parameters and
		// ranges agree with each other.
		assert(m_extents[0] > 0 && m_extents[0] <= 4);
		assert(m_extents[1] > 0);
		assert(format_size <= m_packing.stride);
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
