#include <heatsink/gl/buffer.hpp>

namespace heatsink::gl {
	buffer buffer::immutable(GLenum target, std::size_t size, GLbitfield access) {
		if (size == 0)
			throw exception("gl::buffer", "cannot create immutable buffer with no data.");
		
		return buffer(target, size, nullptr, access);
	}

	buffer::buffer(GLenum target)
	: object<GL_BUFFER>(target), m_immutable{false}, m_base{}, m_size{} {}

	buffer::buffer(GLenum target, std::size_t size, GLenum usage)
	: buffer(target) {
		this->set(size, usage);
	}

	buffer::buffer(const buffer& b, std::size_t offset, std::size_t size)
	: object<GL_BUFFER>(b), m_immutable{b.m_immutable} {
		assert(this->is_valid());
		if (offset + size > b.m_size) {
			make_error_stream("gl::buffer")
				<< "could not make view "
				<< "(offset=" << offset << ", size=" << size << ") "
				<< "out of buffer "
				<< "(size=" << b.m_size << ")." << std::endl;

			throw exception("gl::buffer", "buffer view range out of bounds.");
		}

		m_base = b.m_base + offset;
		m_size = size;
	}

	buffer::buffer(GLenum target, std::size_t size, const void* data, GLbitfield access)
	: object<GL_BUFFER>(target), m_immutable{true}, m_base{}, m_size{size} {
		this->bind();
		glBufferStorage(this->get_target(), (GLsizeiptr)m_size, data, access);
	}

	void buffer::set(std::size_t size, GLenum usage) {
		assert(this->is_valid() && m_base == 0);
		if (this->is_immutable())
			throw exception("gl::buffer", "cannot reallocate immutable buffer.");

		m_size = size;

		this->bind();
		if (m_size != 0)
			glBufferData(this->get_target(), (GLsizeiptr)m_size, nullptr, usage);
	}

	void buffer::invalidate() {
		assert(this->is_valid());
		// No need to do anything if the buffer is empty.
		if (this->is_empty())
			return;

		this->bind();
		glInvalidateBufferSubData(this->get_target(), (GLintptr)m_base, (GLsizeiptr)m_size);
	}

	buffer::view buffer::make_view(std::size_t offset, std::size_t size) {
		assert(this->is_valid());
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
