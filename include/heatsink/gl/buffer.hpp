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
		/**
		 * A subset of a buffer. Buffer subsets have special additional features
		 * in newer OpenGL versions such as range-based binding to units or
		 * vertex arrays. Note that the lifetime of a view is dependent on its
		 * owning `buffer`. Using a view after the buffer is destroyed will
		 * result in undefined behavior.
		 */
		class const_view;
		/**
		 * A mutable subset of a buffer. Also allows portions of the parent
		 * buffer to be updated while leaving other portions unchanged. See
		 * `buffer::const_view`.
		*/
		class view;

		/**
		 * A portion of OpenGL buffer memory mapped into the user-accessible
		 * address space. This allows buffers/views to be modified with direct
		 * memory addresses rather than the "set" or "update" methods. Note that
		 * like `view`, the lifetime of a mapping is dependent on its `buffer`.
		 */
		template<class T>
		class mapping;

	private:
		// A base for the two views to limit repetition.
		template<bool Const>
		class basic_view;

	public:
		/**
		 * Create a buffer backed by immutable storage. This means that the size
		 * of the data cannot be changed after creation; only the `glUpdate*()`
		 * methods can be used to modify the buffer backing. The access bits
		 * specify read, write, dynamic storage, persistent, coherent, etc. This
		 * overload reserves space but does not fill it with any data.
		 */
		static buffer immutable(GLenum mode, std::size_t, GLbitfield access);
		/**
		 * Create an immutable buffer and fill it with data pointed to by the
		 * given iterator range. See the above overload.
		 */
		template<std::contiguous_iterator Iterator>
		static buffer immutable(GLenum mode, Iterator begin, Iterator end, GLbitfield access);

	public:
		/**
		 * Create a new buffer with the given mode. With this overload, no data
		 * will be allocated; `set()` MUST be called before any other methods
		 * are invoked on it.
		 */
		buffer(GLenum);
		/**
		 * Create a new buffer and reserve the given amount of space for it
		 * (such that `update()` can be called immediately). For more details,
		 * see the above overload.
		 */
		buffer(GLenum mode, std::size_t, GLenum usage = GL_STATIC_DRAW);
		/**
		 * Create a new buffer and fill it with the data pointed to by the given
		 * iterator range. The individual value type will be used to deduce the
		 * appropriate data format and type. See the above overload.
		 */
		template<std::contiguous_iterator Iterator>
		buffer(GLenum mode, Iterator begin, Iterator end, GLenum usage = GL_STATIC_DRAW);

	protected:
		// Copy a buffer, but alter parameters appropriate for a `view` with the
		// given range. Used by the `view` constructor.
		buffer(const buffer&, std::size_t offset, std::size_t size);

	private:
		// Create an immutable buffer with the given data and access.
		buffer(GLenum mode, std::size_t, const void* data, GLbitfield access);

	public:
		/**
		 * Reallocate the backing store of this buffer to the given size, and
		 * provide a new usage hint. The size may be different from the current
		 * `size()`. Note that this method may not be called on immutable
		 * buffers. Note that any existing views are invalidated after
		 * reallocation; using them will result in undefined behavior.
		 */
		void set(std::size_t, GLenum usage = GL_STATIC_DRAW);
		/**
		 * Reallocate the buffer store and fill it with the data pointed to by
		 * the given iterator range. See the above overload.
		 */
		template<std::contiguous_iterator Iterator>
		void set(Iterator begin, Iterator end, GLenum usage = GL_STATIC_DRAW);

		/**
		 * Update the backing store of this buffer in-place. The
		 * `std::distance()` of the iterator range must be equal to the current
		 * `size()` of the buffer. This method may be called on a buffer that
		 * was created immutably.
		 */
		template<std::contiguous_iterator Iterator>
		void update(Iterator begin, Iterator end);

		/**
		 * Clear the buffer store to the specified value. The format species how
		 * the data will be stored inside the buffer, copied from a single pixel
		 * with the given format, whose components are specified within the
		 * iterator range.
		 */
		template<std::contiguous_iterator Iterator>
		void clear(GLenum internal_format, Iterator begin, Iterator end, pixel_format);
		/**
		 * Clear the buffer store to the specified value, where the pixel is
		 * specified as a multi-component tensor type. See the above overload.
		 */
		template<tensor T>
		void clear(GLenum internal_format, const T&, pixel_format = pixel_format::from_type<T>());
		/**
		 * Mark the memory region of this buffer as undefined. This will cause
		 * pending operations to use the old data, but client updates will be
		 * performed on a new region of memory, preventing implicit sync.
		 */
		void invalidate();

		/**
		 * Create a view out of a subset of this buffer. See `view::view()`.
		 */
		const_view make_view(std::size_t offset, std::size_t size) const;
		/**
		 * Create a mutable view from this buffer. See the above overload.
		 */
		view make_view(std::size_t offset, std::size_t size);
		/**
		 * Create a mapping out of the entire range represented by this buffer.
		 * A pointer to the data is made available in the specified type, or as
		 * a `GLubyte`/`unsigned char` if none is provided.
		 */
		template<class T = GLubyte>
		mapping<T> map(GLbitfield access);

		/**
		 * Check if a buffer is immutable; that is, created with the
		 * `immutable()` static methods (`glBufferStorage()`). An immutable
		 * buffer cannot have its storage reallocated after it is created; it
		 * can only be updated or cleared.
		 */
		bool is_immutable() const;
		/**
		 * Check if this buffer does not have any data set (that is, its size
		 * is zero). Most operations cannot be performed if this is true; a
		 * `set()` overload must be called first.
		 */
		bool is_empty() const;

		/**
		 * Retrieve the number of bytes held by this buffer. Note that if the
		 * buffer is created with an iterator range / container of size `N`, the
		 * resultant size of the buffer will be `N * sizeof(value_type)`.
		 */
		std::size_t get_size() const;

		/**
		 * Allow the binding of a buffer to be changed at any time. A buffer can
		 * be rebound at any time, to allow it to be used as data for different
		 * kinds of OpenGL operations.
		 */
		using object::rebind;

	protected:
		// Allow subclass access to the base "offset" managed by this buffer.
		std::size_t get_base_offset() const;

	private:
		// Whether the buffer was created with `glBufferStorage()`.
		bool m_immutable;
		// The start of the data managed in this buffer. Used by subclasses.
		std::size_t m_base;
		// The size in bytes of the buffer.
		std::size_t m_size;
	};
}
