#pragma once

#include <cstdlib>
#include <iterator>
#include <span>

#include <heatsink/gl/object.hpp>
#include <heatsink/platform/gl.hpp>

namespace heatsink::gl {
	/**
	 * A basic OpenGL buffer type. This represents the base functionality common
	 * across all types (or `mode`s, as referred to in the class members).
	 * More usable variants will extend from the class and implement type-
	 * specific behavior.
	 */
	class buffer : public object<GL_BUFFER> {
	public:
		class const_view;
		class view;

		template<class T>
		class mapping;

	private:
		template<bool Const>
		class basic_view;

	public:
		static buffer immutable(GLenum mode, std::size_t, GLbitfield access);
		template<std::contiguous_iterator Iterator>
		static buffer immutable(GLenum mode, Iterator begin, Iterator end, GLbitfield access);

	public:
		buffer(GLenum);
		buffer(GLenum mode, std::size_t, GLenum usage = GL_STATIC_DRAW);
		template<std::contiguous_iterator Iterator>
		buffer(GLenum mode, Iterator begin, Iterator end, GLenum usage = GL_STATIC_DRAW);

	protected:
		buffer(const buffer&, std::size_t offset, std::size_t size);

	private:
		buffer(GLenum mode, std::size_t, const void* data, GLbitfield access);

	public:
		void set(std::size_t, GLenum usage = GL_STATIC_DRAW);
		template<std::contiguous_iterator Iterator>
		void set(Iterator begin, Iterator end, GLenum usage = GL_STATIC_DRAW);

		template<std::contiguous_iterator Iterator>
		void update(Iterator begin, Iterator end);

		template<std::contiguous_iterator Iterator>
		void clear(GLenum internal_format, Iterator begin, Iterator end, pixel_format);
		template<tensor T>
		void clear(GLenum internal_format, const T&, pixel_format = pixel_format::from_type<T>());
		void invalidate();

		const_view make_view(std::size_t offset, std::size_t size) const;
		view make_view(std::size_t offset, std::size_t size);
		template<class T = GLubyte>
		mapping<T> map(GLbitfield access);

		bool is_immutable() const;
		bool is_empty() const;

		std::size_t get_size() const;
		using object::rebind;

	protected:
		std::size_t get_base_offset() const;

	private:
		bool m_immutable;
		std::size_t m_base;
		std::size_t m_size;
	};
}
