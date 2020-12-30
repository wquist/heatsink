#include <heatsink/gl/vertex_array.hpp>

#include <cassert>

#include <heatsink/traits/memory.hpp>
#include <heatsink/traits/name.hpp>

namespace heatsink::gl {
	vertex_array::vertex_array()
	: object<GL_VERTEX_ARRAY>() {}

	void vertex_array::set_attribute(const attribute& a, vertex_format f, buffer::const_view v) {
		assert(this->is_valid());
		assert(v.get_target() == GL_ARRAY_BUFFER);

		this->set_attribute(a, f, v, nullptr);
	}

	void vertex_array::set_attribute(const attribute& a, vertex_format f, buffer::const_view v, conversion conv) {
		assert(this->is_valid());
		assert(v.get_target() == GL_ARRAY_BUFFER);

		this->set_attribute(a, f, v, &conv);
	}

	void vertex_array::set_attribute(const attribute& a, std::nullptr_t) {
		assert(this->is_valid());

		this->bind();
		glDisableVertexAttribArray(a.get());
	}

	void vertex_array::set_elements(const buffer& b) {
		assert(this->is_valid());
		assert(b.get_target() == GL_ELEMENT_ARRAY_BUFFER);

		this->bind();
		b.bind();
	}

	void vertex_array::set_elements(std::nullptr_t) {
		assert(this->is_valid());

		this->bind();
		name_traits<GL_BUFFER>::bind(0, GL_ELEMENT_ARRAY_BUFFER);
	}

	void vertex_array::set_attribute(const attribute& a, vertex_format f, buffer::const_view v, conversion* conv) {
		auto type    = f.get_datatype();
		auto extents = f.get_extents();
		auto packing = f.get_packing();
		auto offset  = v.get_offset() + packing.offset;

		auto base    = a.get();
		auto indices = a.get_size();
		assert(!a.is_annotated() || indices == extents[1]);

		this->bind();
		v.bind();

		for (auto i = 0; i != indices; ++i) {
			auto cs = extents[0];
			if (type == GL_DOUBLE) {
				// In a `dvec3`, the components must be uploaded in two pieces;
				// two in the first index, and one in the second.
				if (cs == 3)
					cs = (i % 2) ? 1 : 2;
				// In a `dvec4`, the components can be uploaded evenly; 2 each.
				else if (cs == 4)
					cs = 2;
			}

			auto index = base + i;
			glEnableVertexAttribArray(index);

			if (!conv) {
				glVertexAttribPointer(index, (GLint)cs, type, GL_TRUE, (GLsizei)packing.stride, (GLvoid*)offset);
			} else switch (*conv) {
				case conversion::integer:
					glVertexAttribIPointer(index, (GLint)cs, type, (GLsizei)packing.stride, (GLvoid*)offset);
					break;
				case conversion::floating_point:
					glVertexAttribPointer(index, (GLint)cs, type, GL_FALSE, (GLsizei)packing.stride, (GLvoid*)offset);
					break;
				case conversion::double_precision:
					glVertexAttribLPointer(index, (GLint)cs, type, (GLsizei)packing.stride, (GLvoid*)offset);
					break;
			}

			offset += size_of(type) * cs;
		}
	}
}
