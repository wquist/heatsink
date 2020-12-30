#pragma once

#include <cstdlib>
#include <type_traits>

#include <glm/glm.hpp>

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
		 * The components/array size of the vertex format. The first dimension
		 * describes the component count; that is, the number of elements per
		 * single vector. The second represents the array size, which is also
		 * equal to the number of attribute indices needed to represent the
		 * complete format. For example, a mat3[2] would have extents [3,6].
		 */
		using extents = glm::uvec2;

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
		 * Create a vertex format from an OpenGL type enumeration. This allows
		 * vertex formats to quickly be created from standard FLOAT_VEC3, etc.
		 */
		vertex_format(GLenum);
		/**
		 * Create a vertex format from the given base type, dimensions, and
		 * packing information. The type enumeration must be one of the GL
		 * primitive types (non-vector and non-matrix).
		 */
		vertex_format(GLenum, extents, packing = {});

		/**
		 * Infer the parameters of a vertex format from a structure member
		 * pointer. If `force_array` is `false`, single-dimension primitive
		 * types will be treated as vectors if their size can represent a
		 * vector type, rather than a single value scalar array.
		 */
		template<tensor T, standard_layout Vertex>
		vertex_format(T Vertex::*member, bool force_array = true);

	public:
		/**
		 * Retrieve the component type of this format.
		 */
		GLenum get_datatype() const;
		/**
		 * Retrieve the extents (component and attribute index count) of this
		 * vertex format. See the `extents` type definition.
		 */
		extents get_extents() const;
		/**
		 * Retrieve the stride and offset of this format.
		 */
		packing get_packing() const;

	private:
		// The individual component type.
		GLenum m_datatype;
		// The extents of the components/indices.
		extents m_extents;
		// The field packing within a larger structure.
		packing m_packing;
	};
}

namespace heatsink::gl {
	template<tensor T, standard_layout Vertex>
	vertex_format::vertex_format(T Vertex::*member, bool force_array)
	: m_packing{.stride = sizeof(Vertex), .offset = offset_of(member)} {
		// Find the inner type and dimensions of the given member, since a
		// vertex is always represented as a scalar type (int, float, etc.).
		using value_type = tensor_decay_t<T>;

		constexpr auto value_rank = std::rank_v<value_type>;
		// The format must be represented by either a scalar, a single-dimension
		// array (which may be either a vector or a normal array), or a two-
		// dimensional array represented an array of vector types.
		static_assert(value_rank <= 2);

		constexpr auto datatype   = make_enum_v<std::remove_all_extents_t<value_type>>;
		static_assert(datatype != GL_NONE);

		m_datatype = datatype;
		switch (value_rank) {
			case 2: {
				// The "vector" dimension is the higher one. Assign to a
				// `constexpr` since it cannot be compiler asserted otherwise.
				constexpr auto components = std::extent_v<value_type, 1>;
				static_assert(components <= 4);

				m_extents = glm::vec2(components, std::extent_v<value_type, 0>);
				break;
			}
			case 1: {
				constexpr auto components = std::extent_v<value_type, 0>;
				// If there is only one dimension, it could be either the
				// "vector" or array dimension. The extent is coerced to a
				// vector type if the array size is sufficiently small.
				if (components > 4 || force_array)
					m_extents = glm::vec2(1, components);
				else
					m_extents = glm::vec2(components, 1);

				break;
			}
			default: {
				// The type is a scalar, so both extents are singular.
				m_extents = glm::vec2(1);
			}
		}
	}
}
