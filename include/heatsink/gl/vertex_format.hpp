#pragma once

#include <cstdlib>
#include <type_traits>

#include <heatsink/platform/gl.hpp>
#include <heatsink/traits/enum.hpp>
#include <heatsink/traits/tensor.hpp>

namespace heatsink::gl {
	/**
	 * An OpenGL vertex format, used to define VAO attributes in
	 * `glVertexAttribPointer()`. This includes information on the vertex type
	 * itself, as well as its position in a larger structure or array. Note that
	 * one vertex format can represent multiple attribute locations.
	 */
	class vertex_format {
	public:
		/**
		 * Vertex buffer data may contain data for multiple attributes, in which
		 * case the "packing" of the data must also be specified. A tightly
		 * packed buffer (one attribute per element) needs no stride/offset.
		 * However, if a buffer contains interleaved elements, the offset
		 * (the position of the attribute within the element) and the stride
		 * (the entire size of the element) must be specified.
		 */
		struct packing {
		public:
			// The order of stride/offset is the same as the GL methods.
			std::size_t stride;
			std::size_t offset;
		};

	public:
		/**
		 * Create a vertex format from the given base type, dimensions, and
		 * packing information. The type enumeration must be one of the GL
		 * primitive types (non-vector and non-matrix).
		 */
		vertex_format(GLenum, std::size_t components, std::size_t n, packing = {});
		/**
		 * Create a "simple" (non-array) vertex format. This is like the above
		 * constructor, but allows the component size to be passed with an
		 * assumed array size of `1`. All other parameters are identical.
		 */
		vertex_format(GLenum, std::size_t components, packing = {});

		/**
		 * Infer the parameters of a vertex format from a structure member
		 * pointer. If `force_array` is `false`, single-dimension primitive
		 * types will be treated as vectors if their size can represent a
		 * vector type, rather than a single value scalar array.
		 */
		template<tensor T, standard_layout Vertex>
		vertex_format(T Vertex::*member, bool force_array = false);

	public:
		/**
		 * Retrieve the component type of this format.
		 */
		GLenum get_datatype() const;
		/**
		 * Retrieve the component count of this format.
		 */
		std::size_t get_component_count() const;
		/**
		 * Retrieve the attribute count of this format (the number of attribute
		 * indices needed to represent this format).
		 */
		std::size_t get_index_count() const;

		/**
		 * Retrieve the stride and offset of this format.
		 */
		packing get_packing() const;

	private:
		// The individual component type.
		GLenum m_enum;
		// The extents of the component/index dimensions.
		std::size_t m_components;
		std::size_t m_indices;

		// The field packing within a larger structure.
		packing m_packing;
	};
}

namespace heatsink::gl {
	template<tensor T, standard_layout Vertex>
	vertex_format(T Vertex::*member, bool force_array)
	: m_packing{.stride = sizeof(Vertex), .offset = offset_of(member)} {
		// Find the inner type and dimensions of the given member, since a
		// vertex is always represented as a scalar type (int, float, etc.).
		using value_type = tensor_decay_t<T>;

		constexpr auto value_rank = std::rank_v<value_type>;
		// The format must be represented by either a scalar, a single-dimension
		// array (which may be either a vector or a normal array), or a two-
		// dimensional array represented an array of vector types.
		static_assert(value_rank <= 2);

		constexpr auto datatype = make_enum_v<std::remove_all_extents_t<value_type>>;
		static_assert(datatype != GL_NONE);

		m_enum = datatype;
		switch (value_rank) {
			case 2: {
				// The "vector" dimension is the higher one. Assign to a
				// `constexpr` since it cannot be compiler asserted otherwise.
				constexpr auto components = std::extent_v<value_type, 1>;
				static_assert(components <= 4);

				m_components = components;
				m_indices    = std::extent_v<value_type, 0>;
				break;
			}
			case 1: {
				m_components = std::extent_v<value_type>;
				m_indices    = 1;

				// If there is only one dimension, it could be either the
				// "vector" or array dimension. The extent is coerced to a
				// vector type if the array size is sufficiently small.
				if (m_components > 4 || force_array)
					std::swap(m_components, m_indices);

				break;
			}
			default: {
				m_components = 1;
				m_indices    = 1;
			}
		}
	}
}
