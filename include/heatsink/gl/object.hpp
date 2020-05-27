#pragma once

#include <cassert>
#include <cstdlib>

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
		using name = name_traits<V>;

	public:
		/**
		 * Create a new OpenGL object. This calls the appropriate `glGen*()`
		 * method for the templatize object type. Some objects may take no
		 * arguments, while others will take a bind target enumeration.
		 */
		explicit object()
			requires (name::has_target == false);
		explicit object(GLenum mode)
			requires (name::has_target == true);

		// Objects are noncopyable, since only one reference to an OpenGL state
		// should exist at a time, but are moveable.
		object(object&&) noexcept;
		~object();

		object& operator =(const object&) = delete;
		object& operator =(object&&) noexcept;

	protected:
		// Create an invalid instance of an object.
		object(std::nullptr_t);
		// Allow a valid object to be constructed with the default name. This is
		// only valid for some object types (like framebuffers). Note that of
		// the objects that are default constructible, they all require targets,
		// so only one overload is needed.
		object(std::nullptr_t, GLenum mode)
			requires (name::is_default_constructible);
		// Allow an object to be copied internally. This is usefully for `view`-
		// like classes used in some object types.
		object(const object&) = default;

	public:
		/**
		 * Bind an object to its target. For some objects, this means no
		 * arguments are needed, while others will require an image unit value.
		 */
		void bind() const
			requires (name::has_image_unit == false);
		void bind(std::size_t unit) const
			requires (name::has_image_unit == true);

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
		GLenum get_target() const
			requires (name::has_target == true);

	protected:
		// Permanently change the bind target of this object. This also performs
		// the same functions as `bind()` after the target is changed.
		void rebind(GLenum mode)
			requires (name::has_target == true);

		// Clear the OpenGL name of this object without freeing it.
		void reset();

	private:
		// The OpenGL identifier for this object. Invalid when set to `0`.
		GLuint m_name;
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
	object<V>::object() requires (name::has_target == false)
	: detail::object_target_mixin<V>(), m_name{name::create()} {}

	template<GLenum V>
	object<V>::object(GLenum mode) requires (name::has_target == true)
	: detail::object_target_mixin<V>(), m_name{name::create()} {
		// Set the target after the object has been created so it is left in
		// the correct state in case of an exception.
		detail::object_target_mixin<V>::set_target(mode);
	}

	template<GLenum V>
	object<V>::object(object&& other) noexcept
	: detail::object_target_mixin<V>(std::move(other)), m_name{other.m_name} {
		other.reset();
	}

	template<GLenum V>
	object<V>::~object() {
		if (m_name)
			name::destroy(m_name);
	}

	template<GLenum V>
	object<V>& object<V>::operator =(object&& other) noexcept {
		if (m_name)
			name::destroy(m_name);

		detail::object_target_mixin::operator =(std::move(other));
		m_name = other.m_name;

		other.reset();
		return *this;
	}

	template<GLenum V>
	object<V>::object(std::nullptr_t)
	: detail::object_target_mixin<V>(), m_name{0} {}

	template<GLenum V>
	object<V>::object(std::nullptr_t, GLenum mode) requires (name::is_default_constructible)
	: detail::object_target_mixin<V>(mode), m_name{0} {}

	template<GLenum V>
	void object<V>::bind() const requires (name::has_image_unit == false) {
		assert(this->is_valid());

		// When binding, use the mixin target accessor in case we are binding
		// the default (name `0`) object of this type.
		if constexpr (name::has_target)
			name::bind(m_name, detail::object_target_mixin<V>::get_target());
		else
			name::bind(m_name);
	}

	template<GLenum V>
	void object<V>::bind(std::size_t unit) const requires (name::has_image_unit == true) {
		assert(this->is_valid());

		// When binding, use the mixin target accessor in case we are binding
		// the default (name `0`) object of this type.
		if constexpr (name::has_target)
			name::bind(m_name, detail::object_target_mixin<V>::get_target(), unit);
		else
			name::bind(m_name, unit);
	}

	template<GLenum V>
	bool object<V>::is_valid() const {
		auto is_default = (name::is_default_constructible && m_name == 0);
		return (m_name != 0 || is_default);
	}

	template<GLenum V>
	GLuint object<V>::get() const {
		assert(this->is_valid());
		return m_name;
	}

	template<GLenum V>
	GLenum object<V>::get_target() const requires (name::has_target == true) {
		assert(this->is_valid());
		return detail::object_target_mixin<V>::get_target();
	}

	template<GLenum V>
	void object<V>::rebind(GLenum mode) requires (name::has_target == true) {
		assert(this->is_valid());

		detail::object_target_mixin<V>::set_target(mode);
		// An object that will be rebound normally should not have a bind unit,
		// but check to ensure correct syntax.
		if constexpr (name::has_bind_unit)
			this->bind(0u);
		else
			this->bind();
	}

	template<GLenum V>
	void object<V>::reset() {
		m_name = 0;
		// Reset the target to ensure the object is left in an invalid state.
		detail::object_target_mixin<V>::set_target(GL_NONE);
	}

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
