#pragma once

#include <cassert>
#include <cstdlib>
#include <iterator>
#include <ostream>
#include <type_traits>

#include <glm/glm.hpp>
#include <glm/gtc/vec1.hpp>
#include <glm/gtx/string_cast.hpp>

#include <heatsink/error/debug.hpp>
#include <heatsink/error/exception.hpp>
#include <heatsink/gl/object.hpp>
#include <heatsink/gl/pixel_format.hpp>
#include <heatsink/platform/gl.hpp>
#include <heatsink/traits/memory.hpp>
#include <heatsink/traits/tensor.hpp>
#include <heatsink/traits/texture.hpp>

namespace heatsink::gl {
	/**
	 * A basic OpenGL texture type. This represents all dimensions/special types
	 * of textures, where unique parameters are specified through the texture
	 * `target`. All texture types can be used and modified through the same
	 * basic methods, bar cubemaps, which require special handling of faces.
	 * Note that all texture operations (`set()`, `update()`, etc.) bind to the
	 * first texture unit (unit `0`).
	 */
	class texture : public object<GL_TEXTURE> {
	public:
		/**
		 * An n-dimensional size type. This holds the width, height, and depth
		 * of a texture (or a subset of these measurements, depending on the
		 * rank of the texture). This can be created from the `glm::uvecN`
		 * types, but the dimension must be appropriate for the texture target.
		 */
		class extents;

		/**
		 * A subset of a texture. For array/cubemap textures, a view can also
		 * represent multiple layers or a single slice. Note that the lifetime
		 * of a view is dependent on its owning `texture`. Using a view after
		 * the texture is destroyed will result in undefined behavior. This is
		 * not an "OpenGL Texture View"; instead, it is a heatsink-style view.
		 */
		class const_view;
		/**
		 * A mutable subset of a texture. Additionally, allows portions of the
		 * parent texture to be updated while leaving other portions unchanged.
		 * See `texture::const_view`.
		 */
		class view;

	private:
		// A base for the two views to limit repetition.
		template<bool Const>
		class basic_view;

	public:
		/**
		 * Create a texture backed by immutable storage. This means that the
		 * size of the data cannot be changed after creation; only the
		 * `glUpdate*()` methods can be used to modify the texture backing. The
		 * format specifies the internal representation of the texture; OpenGL
		 * will convert any data passed in `update()` to this format.
		 */
		static texture immutable(GLenum, GLenum ifmt, extents, std::size_t mips = 1);
		/**
		 * Create an immutable texture that can be multisampled. This texture is
		 * also cannot be changed after creation, and additionally cannot be
		 * modified with `update()` either. Its main use is for framebuffers.
		 */
		static texture multisample(GLenum, GLenum ifmt, extents, std::size_t n, bool fix = true);

	public:
		/**
		 * Create a new texture with the given target. With this overload, no
		 * data will be allocated; `set()` MUST be called before any other
		 * methods are invoked on it.
		 */
		texture(GLenum);
		/**
		 * Create a new texture with the given size and internal format. The
		 * space for this texture is reserved such that `update()` can be called
		 * immediately. See the above overload.
		 */
		texture(GLenum, GLenum ifmt, extents, std::size_t mips = 1);
		/**
		 * Create a new texture and fill it with the data pointed to by the
		 * given iterator range. The size and format specified must be
		 * appropriate for the amount of data given. See the above overload.
		 */
		template<std::contiguous_iterator Iterator>
		texture(GLenum, GLenum ifmt, extents, Iterator begin, Iterator end, pixel_format);

	protected:
		// Copy a texture, but alter parameters appropriate for a `view` with
		// the given range. Used by the `view` constructor.
		texture(const texture&, extents offset, extents size);
	
	private:
		// Create an immutable texture with the given target and format.
		texture(GLenum, GLenum ifmt, extents, std::size_t mips);
		// Create a multisample texture with the given target and format.
		texture(GLenum, GLenum ifmt, extents, std::size_t n, bool fix);

	public:
		/**
		 * Reallocate the backing store of this texture to the given size and
		 * format. Note that this method may not be called on immutable
		 * textures. All current mip levels are invalidated and new ones are
		 * created. Additionally, any existing views are invalidated after
		 * reallocation; using them is undefined behavior.
		 */
		void set(GLenum ifmt, extents, std::size_t mips = 1);
		/**
		 * Reallocate the texture store and fill it with the data pointed to by
		 * the given iterator range. The size of the data provided (measured
		 * with `std::distance()`) must be equal to the space given by the size
		 * and format of the texture. Refer to the `gl::size_of()` method for
		 * calculations. See the above overload.
		 */
		template<std::contiguous_iterator Iterator>
		void set(GLenum ifmt, extents, Iterator begin, Iterator end, pixel_format);

		/**
		 * Update the backing store of this texture in-place. The
		 * `std::distance()` of the iterator range must be appropriate for the
		 * texture format and size (see `set()`). This method may be called on
		 * a buffer that was created immutably. If the mip is greater than or
		 * equal to the current number of mip levels, an exception is thrown.
		 */
		template<std::contiguous_iterator Iterator>
		void update(std::size_t mip, Iterator begin, Iterator end, pixel_format);

		/**
		 * Clear the buffer store to the specified value, where the pixel is
		 * specified as a multi-component tensor type. The clear value will be
		 * copied as a single pixel with the given format, whose components are
		 * specified within the iterator range.
		 */
		template<tensor T>
		void clear(std::size_t mip, const T&, pixel_format = pixel_format::from_type<T>());
		/**
		 * Mark the memory region of this texture as undefined. This will cause
		 * OpenGL to orphan and reallocate the storage, so new operations do not
		 * need to wait for synchronization on the old memory.
		 */
		void invalidate(std::size_t mip);

		/**
		 * Create a view out of a subset of this texture. Any overload with a
		 * size of dimension less than or equal to this texture dimension may be
		 * used. See `view::view()`.
		 */
		const_view make_view(extents offset, extents size) const;
		/**
		 * Create a mutable view of this texture. See the above overload.
		 */
		view make_view(extents offset, extents size);

		/**
		 * Check if a texture is immutable; that is, created with the
		 * `immutable()` static method (`glTextureStorage()`). An immutable
		 * texture cannot have its storage reallocated after it is created; it
		 * can only be updated. A multisample texture is also immutable.
		 */
		bool is_immutable() const;
		/**
		 * Check if this texture does not have any data set (that is, its size
		 * is zero across all dimensions). Most operations cannot be performed
		 * if this is true; a `set()` overload must be called first.
		 */
		bool is_empty() const;

		/**
		 * Retrieve the dimensions of this texture at the given mip-map level.
		 * This method will throw and exception if the mip level specified
		 * is greater than the allocated number of mips for the texture.
		 */
		extents get_extents(std::size_t mip = 0) const;
		/**
		 * Retrieve the "rank" of this texture, that is, the overall texture
		 * dimension (1D/2D/3D). Equivalent to `get_extents().get_length()`.
		 */
		std::size_t get_rank() const;
		/**
		 * Retrieve the internal format of this texture. This is the internal
		 * format specified during creation or setting of the texture.
		 */
		GLenum get_format() const;
		/**
		 * Retrieve the current number of active mipmap levels for this texture.
		 * If the texture is not immutable, this number may be changed with the
		 * `set()` methods.
		 */
		std::size_t get_mipmap_count() const;

	protected:
		// Allow subclass access to the base "offset" managed by this texture.
		glm::uvec3 get_base(std::size_t mip = 0) const;

	private:
		// Whether the texture was created with `glTextureStorage()`.
		bool m_immutable;
		// The start of the data managed in this texture. Used by subclasses.
		glm::uvec3 m_base;
		// The dimensions of the texture (unused components are always `1`).
		glm::uvec3 m_extents;

		// The internal format of the texture
		GLenum m_format;
		// The number of mipmap levels (`1` if the texture type is unsupported).
		std::size_t m_levels;
	};

	/**
	 * See forward declaration in `texture`.
	 */
	class texture::extents {
	public:
		/**
		 * Create a new `extents` instance of the given length with each
		 * component set to the zero value.
		 */
		static extents zero(std::size_t length);

	public:
		/**
		 * Create a new `extents` from the equivalent glm vector.
		 */
		extents(glm::uvec1);
		extents(glm::uvec2);
		extents(glm::uvec3);
		
	public:
		/**
		 * Get the full (3-dimensional) `glm` vector type representing this
		 * size. Unused components will be set to the specified fill value.
		 */
		glm::uvec3 get(glm::uvec3::value_type fill = 0) const;

		/**
		 * Get the number of active components represented within this object.
		 * This is called "length" to match the `glm` types.
		 */
		std::size_t get_length() const;

	public:
		/**
		 * Two extents can only be equal if their lengths match, and the values
		 * of the active components are also equal.
		 */
		bool operator ==(const extents&) const;
		bool operator !=(const extents&) const;

		/**
		 * Convert this size into its original `glm` vector type.
		 * `glm::uvecN::length()` must be equivalent to the extent of this size,
		 * or an exception will be thrown.
		 */
		explicit operator glm::uvec1() const;
		explicit operator glm::uvec2() const;
		explicit operator glm::uvec3() const;

	private:
		// The actual value. Unused components are set to `0`.
		glm::uvec3 m_components;
		// The number of active components.
		std::size_t m_length;
	};

	/**
	 * See forward declaration in `texture`.
	 */
	template<bool Const>
	class texture::basic_view : protected texture {
	public:
		/**
		 * A texture extents type is also used to describe the extents of its
		 * views, as well as their offsets.
		 */
		using extents = texture::extents;

	private:
		// Conditionally choose the constness to use in both view types.
		using reference = std::conditional_t<Const, const texture&, texture&>;

	public:
		/**
		 * Create a new view from the entire range of the given texture. This
		 * allows classes that accept both textures and views to do so in the
		 * same overload.
		 */
		basic_view(reference);
		/**
		 * Create a new view from the specified texture, using the given range.
		 * Note that the offset/size instances must have the same length as the
		 * extents of the target texture.
		 */
		basic_view(reference, extents offset, extents size);
		/**
		 * Create a new view from the specified view, using the given range. See
		 * the above overload.
		 */
		basic_view(const basic_view&, extents offset, extents size);

		// Adjust RAII behavior for a view; it should be copiable/non-owning.
		basic_view(const basic_view&) = default;
		~basic_view();

		basic_view& operator =(const basic_view&) = default;

	public:
		/**
		 * Retrieve the offset between the starting location of the owning
		 * texture and the first location represented in this view, adjusted for
		 * the given mip level. See `texture::get_extents()`.
		 */
		extents get_offset(std::size_t mip = 0) const;

		/**
		 * Bind the name of the view as the active texture, for its target. Note
		 * that this still binds the entire range of the parent data; it is up
		 * to the methods using it to only use the range specified by this view.
		 */
		using texture::bind;

		/**
		 * A view implements a subset of the `texture` interface. All methods
		 * function exactly the same as they would on the texture, only on the
		 * range specified.
		 */
		using texture::is_valid;
		using texture::is_immutable;
		using texture::is_empty;

		using texture::get_target;
		using texture::get_extents;
		using texture::get_format;
		using texture::get_mipmap_count;
	};

	/**
	 * See forward declaration in `texture`.
	 */
	class texture::const_view : public texture::basic_view<true> {
	public:
		// Inherit all constructors from `basic_view`.
		using basic_view::basic_view;

	public:
		/**
		 * The `make_view()` method cannot be brought in from `texture`, as only
		 * the const overload is valid within a const view.
		 */
		const_view make_view(extents offset, extents size) const;
	};

	/**
	 * See forward declaration in `texture`.
	 */
	class texture::view : public texture::basic_view<false> {
	public:	
		// Inherit all constructors from `basic_view`.
		using basic_view::basic_view;

	public:
		/**
		 * In addition to the parent methods, a mutable view implements some of
		 * the state-changing methods present in `texture`.
		 */
		using texture::update;
		using texture::clear;
		using texture::invalidate;

		using texture::make_view;
	};

	/**
	 * Calculate the number of bytes needed to represent a texture of the given
	 * size and format.
	 */
	constexpr std::size_t size_of(texture::extents, pixel_format);
}

namespace heatsink::gl {
	template<std::contiguous_iterator Iterator>
	texture::texture(GLenum target, GLenum ifmt, extents es, Iterator begin, Iterator end, pixel_format format)
	: texture(target) {
		// Initialize the texture data with mutable storage.
		this->set(ifmt, es, begin, end, format);
	}

	template<std::contiguous_iterator Iterator>
	void texture::set(GLenum ifmt, extents es, Iterator begin, Iterator end, pixel_format format) {
		using T = typename std::iterator_traits<Iterator>::value_type;
		static_assert(is_tensor_v<T>);

		assert(this->is_valid() && m_base == glm::uvec3(0));
		// If a texture is not immutable, it cannot be multisample.
		if (this->is_immutable())
			throw exception("gl::texture", "cannot reallocate immutable texture.");
		
		if (auto size = std::distance(begin, end) * sizeof(T); size != size_of(es, format)) {
			make_error_stream("gl::texture")
				<< "cannot assign data "
				<< "(size=" << size << ") "
				<< "to texture "
				<< "(extents=" << glm::to_string(es) << ", format=" << to_string(ifmt) << ")." << std::endl;

			throw exception("gl::texture", "data size mismatch.");
		}

		auto t = this->get_target();
		// Cubemap storage can only be reallocated with the no-data `set()`.
		if (texture_traits::is_cubemap(t))
			throw exception("gl::texture", "cannot reallocate cubemap with texture data.");

		auto rank = texture_traits::rank(t);
		if (rank != es.get_length()) {
			make_error_stream("gl::texture")
				<< "cannot assign "
				<< es.get_length() << "-dimensional "
				<< "data to "
				<< rank << "-dimensional "
				<< "texture." << std::endl;

			throw exception("gl::texture", "data dimension mismatch.");
		}

		// Like buffers, allocating to `0` bytes has no specific rules.
		if (es == extents::zero(rank))
			return;
		// An extents cannot have only some dimensions be `0` (all or nothing).
		if (auto zeroes = glm::equal(es.get(1), glm::uvec3(0)); glm::any(zeroes))
			throw exception("gl::texture", "invalid texture extents.");

		m_extents = es.get(1);
		m_format  = ifmt;
		m_levels  = 1;

		auto [x, y, z] = m_extents;
		auto pfmt  = format.get();
		auto ptype = format.get_datatype();

		this->bind(0);
		glTexParameteri(t, GL_TEXTURE_MAX_LEVEL, m_levels - 1);

		switch (rank) {
			case 1: glTexImage1D(t, 0, m_format, x,       0, pfmt, ptype, address_of(*begin)); break;
			case 2: glTexImage2D(t, 0, m_format, x, y,    0, pfmt, ptype, address_of(*begin)); break;
			case 3: glTexImage3D(t, 0, m_format, x, y, z, 0, pfmt, ptype, address_of(*begin)); break;
		}
	}

	template<std::contiguous_iterator Iterator>
	void texture::update(std::size_t mip, Iterator begin, Iterator end, pixel_format format) {
		using T = typename std::iterator_traits<Iterator>::value_type;
		static_assert(is_tensor_v<T>);

		assert(this->is_valid());
		if (mip >= m_levels)
			throw exception("gl::texture", "mipmap level out of bounds.");

		auto es = this->get_extents(mip);
		if (auto size = std::distance(begin, end) * sizeof(T); size != size_of(es, format)) {
			make_error_stream("gl::texture")
				<< "cannot assign data "
				<< "(size=" << size << ") "
				<< "to texture "
				<< "(extents=" << glm::to_string(es) << ", format=" << to_string(ifmt) << ")." << std::endl;

			throw exception("gl::texture", "data size mismatch.");
		}

		auto t = this->get_target();
		if (texture_traits::is_multisample(t))
			throw exception("gl::texture", "cannot update multisample texture directly.");
		// If this is a cubemap, only single-face views should be updated.
		if (texture_traits::is_cubemap(t) && m_extents.z != 1)
			throw exception("gl::texture", "cannot update multiple cubemap faces simultaneously.");

		if (this->is_empty())
			return;

		auto rank = texture_traits::rank(t);
		if (t == GL_TEXTURE_CUBE_MAP) {
			// Normal cube maps must be treated as separate 2D textures, based
			// on the current offset of this view. Note that while the 3D
			// variant can be used in OpenGL 4.5, the old method is used to
			// support all version, including 3.3 - 4.4.
			t = GL_TEXTURE_CUBE_MAP_POSITIVE_X + m_base.z;
			rank = 2;
		}

		auto [bx, by, bz] = this->get_base(mip);
		auto [sx, sy, sz] = es.get(1);
		auto pfmt  = format.get();
		auto ptype = format.get_datatype();

		this->bind(0);
		switch (rank) {
			case 1: glTexSubImage1D(t, mip, bx,         sx,         pfmt, ptype, address_of(*begin)); break;
			case 2: glTexSubImage2D(t, mip, bx, by,     sx, sy,     pfmt, ptype, address_of(*begin)); break;
			case 3: glTexSubImage3D(t, mip, bx, by, bz, sx, sy, sz, pfmt, ptype, address_of(*begin)); break;
		}
	}

	template<tensor T>
	void texture::clear(std::size_t mip, const T& t, pixel_format format) {
		assert(this->is_valid());
		if (mip >= m_levels)
			throw exception("gl::texture", "mipmap level out of bounds.");

		// Check for errors as normal, but do nothing if empty.
		if (this->is_empty())
			return;

		auto [bx, by, bz] = this->get_base(mip);
		auto [sx, sy, sz] = this->get_extents(mip).get(1);
		auto pfmt  = format.get();
		auto ptype = format.get_datatype();

		glClearTexSubImage(this->get(), mip, bx, by, bz, sx, sy, sz, pfmt, ptype, address_of(*begin));
	}

	template<bool Const>
	texture::basic_view<Const>::basic_view(reference other)
	: texture(other, extents::zero(other.get_rank()), other.get_extents()) {}

	template<bool Const>
	texture::basic_view<Const>::basic_view(reference other, extents offset, extents size)
	: texture(other, offset, size) {}

	template<bool Const>
	texture::basic_view<Const>::basic_view(const basic_view& other, extents offset, extents size)
	: texture(other, offset, size) {}

	template<bool Const>
	texture::basic_view<Const>::~basic_view() {
		this->reset();
	}

	template<bool Const>
	texture::basic_view<Const>::extents texture::basic_view<Const>::get_offset(std::size_t mip) const {
		assert(this->is_valid());
		return this->get_base(mip);
	}
}
