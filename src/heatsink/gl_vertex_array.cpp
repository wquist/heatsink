#include <heatsink/gl/vertex_array.hpp>

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
		// FIXME: handle multi-index attributes.
		assert(f.get_index_count() == 1);

		auto index   = a.get();
		auto size    = f.get_component_count();
		auto type    = f.get_datatype();
		auto packing = f.get_packing();
		auto offset  = v.get_offset() + packing.offset;

		this->bind();
		v.bind();

		glEnableVertexAttribArray(index);
		if (!c) {
			glVertexAttribPointer(index, size, type, GL_TRUE, packing.stride, (GLvoid*)offset);
		} else switch (*c) {
			case conversion::integer:
				glVertexAttribIPointer(index, size, type, packing.stride, (GLvoid*)offset);
				break;
			case conversion::floating_point:
				glVertexAttribPointer(index, size, type, GL_FALSE, packing.stride, (GLvoid*)offset);
				break;
			case conversion::double_precision:
				glVertexAttribLPointer(index, size, type, packing.stride, (GLvoid*)offset);
				break;
		}
	}
}
