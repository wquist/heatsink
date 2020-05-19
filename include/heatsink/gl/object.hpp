#pragma once

#include <heatsink/platform/gl.hpp>
#include <heatsink/traits/name.hpp>

namespace heatsink::gl {
	namespace detail {
		// A mixin object type to provide a bind target enumeration if needed.
		template<GLenum V, bool Target = name_traits<V>::has_target>
		class object_target_mixin;
	}

	/**
	 * A managed wrapper around the state that makes up an OpenGL object. This
	 * includes the object name, or its identifier, and the object target, the
	 * parameters that describe how it should be bound.
	 */
	template<GLenum V>
	class object : private detail::object_target_mixin<V> {
	private:
		// Provide an alias to the traits used throughout the class.
		template<GLenum VV = V>
		using traits = name_traits<VV>;

	public:
		/**
		 * Create a new OpenGL object. This calls the appropriate `glGen*()`
		 * method for the templatize object type. Some objects may take no
		 * arguments, while others will take a bind target enumeration.
		 */
		template<GLenum VV = V> requires (traits<VV>::has_target == false)
		explicit object();
		template<GLenum VV = V> requires (traits<VV>::has_target == true)
		explicit object(GLenum mode);

		// Objects are noncopyable, since only one reference to an OpenGL state
		// should exist at a time, but are moveable.
		object(object&&) noexcept;
		~object();

		object& operator =(const object&) = delete;
		object& operator =(object&&) noexcept;

	protected:
		// Create an invalid instance of an object.
		object(std::nullptr_t);
		// Allow an object to be copied internally. This is usefully for `view`-
		// like classes used in some object types.
		object(const object&) = default;
		// Allow a valid object to be constructed with the default name. This is
		// only valid for some object types (like framebuffers). Note that of
		// the objects that are default constructible, they all require targets,
		// so only one overload is needed.
		template<GLenum VV = V> requires (traits<VV>::is_default_constructible)
		object(std::nullptr_t, GLenum mode);

	public:
		/**
		 * Bind an object to its target. For some objects, this means no
		 * arguments are needed, while others will require an image unit value.
		 */
		template<GLenum VV = V> requires (traits<VV>::has_bind_unit == false)
		void bind() const;
		template<GLenum VV = V> requires (traits<VV>::has_bind_unit == true)
		void bind(std::size_t unit) const;

		/**
		 * Check if the object instance is valid. An object should be valid
		 * unless it has thrown an exception or been moved from. Calling any
		 * member functions on an invalid object is undefined behavior, although
		 * it is likely to raise an assertion.
		 */
		bool is_valid() const;
		/**
		 * Retrieve the OpenGL name (the instance identifier) for this object.
		 * Use with caution; the identifier is still managed by the instance,
		 * and changes made directly to the identifier may not be reflected.
		 * Note that an assertion is raised if `is_valid()` would equal `false`.
		 */
		GLuint get() const;
		/**
		 * Retrieve the OpenGL mode this object was created with. This method
		 * is only valid for object types that have bind targets.
		 */
		
		GLenum get_target() const;
	};

	// An empty non-variable object. See the above forward declaration.
	template<GLenum V>
	class detail::object_target_mixin<V, false> {};

	// A variable object with a bind target. See the above forward declaration.
	template<GLenum V>
	class detail::object_target_mixin<V, true> {
	public:
		// Create an object with a bind target enumeration.
		object_target_mixin(GLenum mode);

	protected:
		// Allow an internal constructor to match that of `object<V, false>`.
		object_target_mixin();

	public:
		// Retrieve the target this object was constructed with.
		GLenum get_target() const;
		// Update the bind target for this object.
		void set_target(GLenum);

	private:
		// The OpenGL bind target for this object.
		GLenum m_target;
	};
}

namespace heatsink::gl {
	template<GLenum V>
	detail::object_target_mixin<V, true>::object_target_mixin(GLenum mode)
	: m_target{mode} {}

	template<GLenum V>
	detail::object_target_mixin<V, true>::object_target_mixin()
	: m_target{GL_NONE} {}

	template<GLenum V>
	GLenum detail::object_target_mixin<V, true>::get_target() const {
		return m_target;
	}

	template<GLenum V>
	void detail::object_target_mixin<V, true>::set_target(GLenum mode) {
		m_target = mode;
	}
}
