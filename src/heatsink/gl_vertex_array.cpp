#include <heatsink/gl/vertex_array.hpp>

#include <heatsink/traits/memory.hpp>
#include <heatsink/traits/name.hpp>

namespace heatsink::gl {
	vertex_array()
	: object<GL_VERTEX_ARRAY>() {}

	void vertex_array::set_attribute(const attribute& a, vertex_format f, buffer::const_view v) {
		assert(this->is_valid());
		assert(v.get_target() == GL_ARRAY_BUFFER);

		this->set_attribute(a, f, v, nullptr);
	}

	void vertex_array::set_attribute(const attribute& a, vertex_format f, buffer::const_view v, conversion c) {
		assert(this->is_valid());
		assert(v.get_target() == GL_ARRAY_BUFFER);

		this->set_attribute(a, f, v, &c);
	}

	void vertex_array::set_attribute(const attribute& a, std::nullptr_t) {
		assert(this->is_valid());

		this->bind();
		glDisableVertexAttribArray(a.get());
	}

	void vertex_array::set_elemenets(const buffer& b) {
		assert(this->is_valid());
		assert(b.get_target() == GL_ELEMENT_ARRAY_BUFFER);

		this->bind();
		b->bind();
	}

	void vertex_array::set_elemenets(std::nullptr_t) {
		assert(this->is_valid());

		this->bind();
		name_traits<GL_BUFFER>::bind(0, GL_ELEMENT_ARRAY_BUFFER);
	}

	void vertex_array::set_attribute(const attribute& a, vertex_format f, buffer::const_view v, conversion* c) {
		auto index   = a.get();
		auto type    = f.get_datatype();
		auto packing = f.get_packing();
		auto offset  = v.get_offset() + packing.offset;

		this->bind();
		v.bind();

		for (auto i = 0; i != f.get_index_count(); ++i) {
			auto size = f.get_component_count();
			if (type == GL_DOUBLE) {
				// In a `dvec3`, the components must be uploaded in two pieces;
				// two in the first index, and one in the second.
				if (size == 3)
					size = (i % 2) ? 1 : 2;
				// In a `dvec4`, the components can be uploaded evenly; 2 each.
				else if (size == 4)
					size = 2;
			}

			glEnableVertexAttribArray(index + i);
			if (!c) {
				glVertexAttribPointer(index + i, size, type, GL_TRUE, packing.stride, (GLvoid*)offset);
			} else switch (*c) {
				case conversion::integer:
					glVertexAttribIPointer(index + i, size, type, packing.stride, (GLvoid*)offset);
					break;
				case conversion::floating_point:
					glVertexAttribPointer(index + i, size, type, GL_FALSE, packing.stride, (GLvoid*)offset);
					break;
				case conversion::double_precision:
					glVertexAttribLPointer(index + i, size, type, packing.stride, (GLvoid*)offset);
					break;
			}

			offset += size_of(type) * size;
		}
	}
}
