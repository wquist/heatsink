#pragma once

#include <cstdlib>
#include <iterator>
#include <type_traits>

#include <glm/glm.hpp>
#include <glm/ext/vector_uint1.hpp>

#include <heatsink/gl/object.hpp>
#include <heatsink/gl/pixel_format.hpp>
#include <heatsink/platform/gl.hpp>
#include <heatsink/traits/tensor.hpp>

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
		glm::uvec3 get_base() const;

	private:
		// Whether the texture was created with `glTextureStorage()`.
		bool m_immutable;
		// The start of the data managed in this texture. Used by subclasses.
		glm::uvec3 m_base;
		// The dimensions of the texture.
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
		 * texture and the first location represented in this view.
		 */
		extents get_offset() const;

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
}

namespace heatsink::gl {
	//
}
