#pragma once

#include <cassert>
#include <cstdlib>
#include <iterator>
#include <type_traits>

#include <heatsink/error/exception.hpp>
#include <heatsink/gl/object.hpp>
#include <heatsink/gl/pixel_format.hpp>
#include <heatsink/platform/gl.hpp>
#include <heatsink/traits/format.hpp>
#include <heatsink/traits/memory.hpp>

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
		template<standard_layout T>
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
		template<standard_layout T = GLubyte>
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
		std::size_t get_base() const;

	private:
		// Whether the buffer was created with `glBufferStorage()`.
		bool m_immutable;
		// The start of the data managed in this buffer. Used by subclasses.
		std::size_t m_base;
		// The size in bytes of the buffer.
		std::size_t m_size;
	};

	template<bool Const>
	class buffer::basic_view : protected buffer {
	private:
		// Conditionally choose the constness to use in both view types.
		using reference = std::conditional_t<Const, const buffer&, buffer&>;

	public:
		/**
		 * Create a new view from the entire range of the given buffer. This
		 * allows classes that accept both buffers and views to do so in the
		 * same overload.
		 */
		basic_view(reference);
		/**
		 * Create a new view from the specified buffer, using the given range.
		 * Note that the offset/size do not have to be aligned to the boundaries
		 * of the data type that was used to set/update it; the base `buffer`
		 * class works solely on byte boundaries.
		 */
		basic_view(reference, std::size_t offset, std::size_t size);
		/**
		 * Create a new view from the specified view, using the given range. See
		 * the above overload.
		 */
		basic_view(const basic_view&, std::size_t offset, std::size_t size);

		// Adjust RAII behavior for a view; it should be copiable/non-owning.
		basic_view(const basic_view&) = default;
		~basic_view();

		basic_view& operator =(const basic_view&) = default;

	public:
		/**
		 * Retrieve the offset between the start of the owning buffer and the
		 * first byte represented in this view.
		 */
		std::size_t get_offset() const;

		/**
		 * Bind the name of the view as the active buffer, for its target. Note
		 * that this still binds the entire range of the parent data; it is up
		 * to the methods using it to only use the range specified by this view.
		 */
		using buffer::bind;

		/**
		 * A view implements a subset of the `buffer` interface. All methods
		 * function exactly the same as they would on the buffer, only on the
		 * range specified.
		 */
		using buffer::is_valid;
		using buffer::is_immutable;
		using buffer::is_empty;
		
		using buffer::get_target;
		using buffer::get_size;
	};

	/**
	 * See forward declaration in `buffer`.
	 */
	class buffer::const_view : public buffer::basic_view<true> {
	public:
		// Inherit all constructors from `basic_view`.
		using basic_view::basic_view;

	public:
		/**
		 * The `make_view()` method cannot be brought in from `buffer`, as only
		 * the const overload is valid within a const view.
		 */
		const_view make_view(std::size_t offset, std::size_t size) const;
	};

	/**
	 * See forward declaration in `buffer`.
	 */
	class buffer::view : public buffer::basic_view<false> {
	public:
		// Inherit all constructors from `basic_view`.
		using basic_view::basic_view;

	public:
		/**
		 * In addition to the parent methods, a mutable view implements some
		 * additional state-changing methods from `buffer`.
		 */
		using buffer::update;
		using buffer::clear;
		using buffer::invalidate;

		using buffer::rebind;
		using buffer::make_view;
		using buffer::map;
	};

	/**
	 * See forward declaration in `buffer`. Note that this extends from `view`
	 * instead of `buffer` as it simplifies creating a mapping from either.
	 */
	template<standard_layout T>
	class buffer::mapping : private basic_view<true> {
	public:
		/**
		 * Provide STL-like type aliases for the `Container` concept.
		 */
		using value_type      = T;
		using reference       = T&;
		using const_reference = const T&;
		using iterator        = T*;
		using const_iterator  = const T*;
		using difference_type = std::ptrdiff_t;
		using size_type       = std::size_t;

	public:
		/**
		 * Create a new mapping out of the specified constant buffer view. This
		 * implicitly specifies `GL_MAP_READ_BIT` for the access. Note that a
		 * mapping created from a constant view may not be writable.
		 */
		mapping(const const_view&, GLbitfield access);
		/**
		 * Create a new mutable mapping from the specified buffer view. The view
		 * may be readable, writable, or both. Note, though, that a read-only
		 * mapping has exception rules as if created from a `const_view`.
		 */
		mapping(const view&, GLbitfield access);

		// A mapping must manage the lifetime of its data; this means that it
		// cannot be copied, but can be moved.
		mapping(const mapping&) = delete;
		mapping(mapping&&) noexcept;
		~mapping();

		mapping& operator =(const mapping&) = delete;
		mapping& operator =(mapping&&) noexcept;

	private:
		// Allow constant and mutable views to be constructed the same way.
		mapping(const buffer&, GLbitfield access);

	public:
		/**
		 * Synchronize a data write in client memory with OpenGL/GPU memory. The
		 * buffer must have been mapped as explicitly flushable, else this
		 * method will raise an assertion.
		 */
		void flush() const;

		/**
		 * Check if the mapping instance is valid. Separate from the buffer, a
		 * mapping can be invalid from a thrown exception during construction.
		 * Calling any member functions on an invalid mapping is undefined
		 * behavior, although it is likely to raise an assertion.
		 */
		bool is_valid() const;

		/**
		 * Retrieve an iterator to the front and back of the mapping, STL style.
		 * If the mapping was not created with read access, these methods will
		 * raise an exception.
		 */
		const_iterator begin() const;
		const_iterator end() const;
		/**
		 * Retrieve a mutable iterator to the front and back of the mapping. If
		 * the mapping was not created with write access, these methods will
		 * raise an exception.
		 */
		iterator begin();
		iterator end();

		/**
		 * Override the `view` size methods to provide a variant that takes into
		 * account the mapping data type. That is, this will return
		 * `view::size() / sizeof(T)`.
		 */
		std::size_t get_size() const;

		/**
		 * Retrieve a raw pointer to the start of the mapped data. This method
		 * has the same exception rules as the `begin()`/`end()` overloads.
		 */
		const T* get_data() const;
		/**
		 * Retrieve a mutable pointer to the mapped data. See above overload.
		 */
		T* get_data();

		/**
		 * Calculate the size of the view in terms of the mapping data type.
		 * This provides an STL-consistent variant of `get_size()`.
		 */
		std::size_t size() const;
		/**
		 * Retrieve a pointer to the mapping data. This provides STL-consistent
		 * variants of `get_data()`.
		 */
		const T* data() const;
		T* data();

	private:
		// The data pointer returned by the OpenGL map.
		T* m_data;
		// The access parameters, for checking read/write validity.
		GLbitfield m_access;
	};
}

namespace heatsink::gl {
	template<std::contiguous_iterator Iterator>
	buffer buffer::immutable(GLenum mode, Iterator begin, Iterator end, GLbitfield access) {
		using T = typename std::iterator_traits<Iterator>::value_type;
		static_assert(std::is_standard_layout_v<T>);

		auto size = std::distance(begin, end) * sizeof(T);
		assert(size > 0);

		return buffer(mode, size, address_of(*begin), access);
	}

	template<std::contiguous_iterator Iterator>
	buffer::buffer(GLenum mode, Iterator begin, Iterator end, GLenum usage)
	: buffer(mode) {
		this->set(begin, end, usage);
	}

	template<std::contiguous_iterator Iterator>
	void buffer::set(Iterator begin, Iterator end, GLenum usage) {
		using T = typename std::iterator_traits<Iterator>::value_type;
		static_assert(std::is_standard_layout_v<T>);

		assert(this->is_valid() && !this->is_immutable());
		assert(m_base == 0);

		m_size = std::distance(begin, end) * sizeof(T);
		assert(m_size > 0);

		this->bind();
		glBufferData(this->get_target(), m_size, address_of(*begin), usage);
	}

	template<std::contiguous_iterator Iterator>
	void buffer::update(Iterator begin, Iterator end) {
		using T = typename std::iterator_traits<Iterator>::value_type;
		static_assert(std::is_standard_layout_v<T>);

		assert(this->is_valid() && !this->is_empty());
		assert(std::distance(begin, end) * sizeof(T) == m_size);

		this->bind();
		glBufferSubData(this->get_target(), m_base, m_size, address_of(*begin));
	}

	template<std::contiguous_iterator Iterator>
	void buffer::clear(GLenum internal_format, Iterator begin, Iterator end, pixel_format format) {
		using T = typename std::iterator_traits<Iterator>::value_type;
		static_assert(std::is_standard_layout_v<T>);

		assert(this->is_valid() && !this->is_empty());
		assert(format_traits::is_sized(internal_format));
		assert(std::distance(begin, end) * sizeof(T) == format.get_size());

		auto itype = format_traits::underlying_datatype(internal_format);
		auto isize = size_of(itype);

		// A packed type is represented by a single `itype` regardless of its
		// extent (component count). Check to determine the appopriate size.
		auto pixel_size = (is_packed(itype)) ? isize : isize * format_traits::extent(internal_format);
		assert(!(m_base % pixel_size) && !(m_size % pixel_size));

		auto pfmt  = format.get();
		auto ptype = format.get_datatype();

		this->bind();
		glClearBufferSubData(this->get_target(), internal_format, m_base, m_size, pfmt, ptype, address_of(*begin));
	}

	template<tensor T>
	void buffer::clear(GLenum internal_format, const T& value, pixel_format format) {
		auto* data = &value;
		this->clear(internal_format, data, data + 1, format);
	}

	template<standard_layout T>
	buffer::mapping<T> buffer::map(GLbitfield access) {
		assert(this->is_valid() && !this->is_empty());
		return mapping<T>(*this, access);
	}

	template<bool Const>
	buffer::basic_view<Const>::basic_view(reference other)
	: buffer(other, 0, other.get_size()) {}

	template<bool Const>
	buffer::basic_view<Const>::basic_view(reference other, std::size_t offset, std::size_t size)
	: buffer(other, offset, size) {}

	template<bool Const>
	buffer::basic_view<Const>::basic_view(const basic_view& other, std::size_t offset, std::size_t size)
	: buffer(other, offset, size) {}

	template<bool Const>
	buffer::basic_view<Const>::~basic_view() {
		// This will invalidate the referenced name before it can be deleted by
		// the parent buffer constructor.
		this->reset();
	}

	template<bool Const>
	std::size_t buffer::basic_view<Const>::get_offset() const {
		assert(this->is_valid());
		return buffer::get_base();
	}

	template<standard_layout T>
	buffer::mapping<T>::mapping(const const_view& other, GLbitfield access)
	: mapping<T>((const buffer&)other, access | GL_MAP_READ_BIT) {
		assert(!(m_access & GL_MAP_WRITE_BIT));
	}

	template<standard_layout T>
	buffer::mapping<T>::mapping(const view& other, GLbitfield access)
	: mapping<T>((const buffer&)other, access | GL_MAP_WRITE_BIT) {}

	template<standard_layout T>
	buffer::mapping<T>::mapping(mapping&& other) noexcept
	: basic_view(std::move(other)), m_data{other.m_data} {
		other.m_data = nullptr;
	}

	template<standard_layout T>
	buffer::mapping<T>::~mapping() {
		if (m_data) {
			this->bind();
			glUnmapBuffer(this->get_target());
		}
	}

	template<standard_layout T>
	buffer::mapping<T>& buffer::mapping<T>::operator =(mapping&& other) noexcept {
		basic_view(std::move(other));
		if (m_data) {
			this->bind();
			glUnmapBuffer(this->get_target());
		}

		m_data = other.m_data;

		other.m_data = nullptr;
		return *this;
	}

	template<standard_layout T>
	buffer::mapping<T>::mapping(const buffer& other, GLbitfield access)
	: basic_view<true>(other), m_access{access} {
		// The view should be aligned relative to the size of the mapping type.
		auto offset = this->get_offset();
		assert(!(offset % sizeof(T)) && !(basic_view::get_size() % sizeof(T)));

		this->bind();
		m_data = glMapBufferRange(this->get_target(), offset, basic_view::get_size(), access);
		
		// This likely occurs because a view of the buffer is already mapped.
		if (!m_data)
			throw exception("gl::buffer::mapping", "could not map buffer data.");
	}

	template<standard_layout T>
	void buffer::mapping<T>::flush() const {
		assert(this->is_valid());

		this->bind();
		// Use `view::get_size()`; the mapping `size()` is no longer in bytes.
		glFlushMappedBufferRange(this->get_target(), this->get_offset(), basic_view::get_size());
	}

	template<standard_layout T>
	bool buffer::mapping<T>::is_valid() const {
		return (basic_view::is_valid() && m_data != nullptr);
	}

	template<standard_layout T>
	typename buffer::mapping<T>::const_iterator buffer::mapping<T>::begin() const {
		assert(this->is_valid());
		assert(m_access & GL_MAP_READ_BIT);

		return m_data;
	}

	template<standard_layout T>
	typename buffer::mapping<T>::const_iterator buffer::mapping<T>::end() const {
		assert(this->is_valid());
		assert(m_access & GL_MAP_READ_BIT);
		
		return m_data;
	}

	template<standard_layout T>
	typename buffer::mapping<T>::iterator buffer::mapping<T>::begin() {
		assert(this->is_valid());
		assert(m_access & GL_MAP_WRITE_BIT);

		return m_data;
	}

	template<standard_layout T>
	typename buffer::mapping<T>::iterator buffer::mapping<T>::end() {
		assert(this->is_valid());
		assert(m_access & GL_MAP_WRITE_BIT);

		return (m_data + this->get_size());
	}

	template<standard_layout T>
	std::size_t buffer::mapping<T>::get_size() const {
		assert(this->is_valid());
		return (basic_view::get_size() / sizeof(T));
	}

	template<standard_layout T>
	std::size_t buffer::mapping<T>::size() const {
		return this->get_size();
	}

	template<standard_layout T>
	const T* buffer::mapping<T>::get_data() const {
		assert(this->is_valid());
		assert(m_access & GL_MAP_READ_BIT);

		return m_data;
	}

	template<standard_layout T>
	T* buffer::mapping<T>::get_data() {
		assert(this->is_valid());
		assert(m_access & GL_MAP_WRITE_BIT);

		return m_data;
	}

	template<standard_layout T>
	const T* buffer::mapping<T>::data() const {
		return this->get_data();
	}

	template<standard_layout T>
	T* buffer::mapping<T>::data() {
		return this->get_data();
	}
}
