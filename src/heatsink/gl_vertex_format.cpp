#include <heatsink/gl/vertex_format.hpp>

#include <cassert>

#include <heatsink/traits/memory.hpp>

namespace heatsink::gl {
	vertex_format::vertex_format(GLenum datatype, std::size_t components, std::size_t n, packing pack)
	: m_datatype{datatype}, m_components{components}, m_indices{n}, m_packing{pack} {
		// Calculate the format stride before modifying index/component values.
		auto format_size = size_of(m_datatype) * m_components * m_indices;
		if (m_packing.stride == 0)
			m_packing.stride = format_size;

		// If the format uses a double type, each component takes up twice as
		// much space. Because an attribute slot can only hold four float
		// values, only two double components can be represented per index.
		if (m_datatype == GL_DOUBLE)
			m_indices *= 2;

		// Check some basic constraints to ensure the passed parameters and
		// ranges agree with each other.
		assert(m_components && m_components <= 4);
		assert(m_indices > 0);
		assert(format_size <= m_packing.stride);
	}

	vertex_format::vertex_format(GLenum datatype, std::size_t components, packing pack)
	: vertex_format(datatype, components, 1, pack) {}

	GLenum vertex_format::get_datatype() const {
		return m_datatype;
	}

	std::size_t vertex_format::get_component_count() const {
		return m_components;
	}

	std::size_t vertex_format::get_index_count() const {
		return m_indices;
	}

	vertex_format::packing vertex_format::get_packing() const {
		return m_packing;
	}
}
