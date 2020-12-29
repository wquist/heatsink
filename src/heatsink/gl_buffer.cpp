#include <heatsink/gl/buffer.hpp>

namespace heatsink::gl {
	buffer buffer::immutable(GLenum mode, std::size_t size, GLbitfield access) {
		assert(size > 0);
		return buffer(mode, size, nullptr, access);
	}

	buffer::buffer(GLenum mode)
	: object<GL_BUFFER>(mode), m_immutable{false}, m_base{}, m_size{} {}

	buffer::buffer(GLenum mode, std::size_t size, GLenum usage)
	: object<GL_BUFFER>(mode), m_immutable{false}, m_base{} {
		this->set(size, usage);
	}

	buffer::buffer(const buffer& b, std::size_t offset, std::size_t size)
	: object<GL_BUFFER>(b), m_immutable{b.m_immutable}, m_base{b.m_base + offset}, m_size{size} {
		assert(this->is_valid());
		assert(m_base + m_size <= b.m_base + b.m_size);
	}

	buffer::buffer(GLenum mode, std::size_t size, const void* data, GLbitfield access)
	: object<GL_BUFFER>(mode), m_immutable{true}, m_base{}, m_size{size} {
		this->bind();
		glBufferStorage(this->get_target(), (GLsizeiptr)m_size, data, access);
	}

	void buffer::set(std::size_t size, GLenum usage) {
		assert(this->is_valid() && !this->is_immutable());
		assert(m_base == 0 && size > 0);

		m_size = size;

		this->bind();
		glBufferData(this->get_target(), (GLsizeiptr)m_size, nullptr, usage);
	}

	void buffer::invalidate() {
		assert(this->is_valid() && !this->is_empty());

		this->bind();
		glInvalidateBufferSubData(this->get_target(), (GLintptr)m_base, (GLsizeiptr)m_size);
	}

	buffer::view buffer::make_view(std::size_t offset, std::size_t size) {
		assert(this->is_valid() && !this->is_empty());
		return view(*this, offset, size);
	}

	bool buffer::is_immutable() const {
		assert(this->is_valid());
		return m_immutable;
	}

	bool buffer::is_empty() const {
		assert(this->is_valid());
		return (m_size == 0);
	}

	std::size_t buffer::get_size() const {
		assert(this->is_valid());
		return m_size;
	}

	std::size_t buffer::get_base() const {
		return m_base;
	}

	buffer::const_view buffer::const_view::make_view(std::size_t offset, std::size_t size) const {
		return buffer::make_view(offset, size);
	}
}
