#include <heatsink/gl/vertex_format.hpp>

#include <cassert>

#include <heatsink/traits/memory.hpp>

namespace heatsink::gl {
	vertex_format::vertex_format(GLenum datatype, std::size_t components, std::size_t n, packing pack)
	: m_enum{datatype}, m_components{components}, m_indices{n}, m_packing{pack} {
		auto format_size = size_of(m_enum) * m_components * m_indices;
		if (m_packing.stride == 0)
			m_packing.stride = format_size;

		// Check some basic constraints to ensure the passed parameters and
		// ranges agree with each other.
		assert(m_components > 0 && m_indices > 0);
		assert(format_size <= m_packing.stride);
	}

	vertex_format::vertex_format(GLenum datatype, std::size_t components, packing pack)
	: vertex_format(datatype, components, 1, pack) {}

	GLenum vertex_format::get_datatype() const {
		return m_enum;
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
