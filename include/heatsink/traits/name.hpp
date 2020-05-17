#pragma once

#include <heatsink/platform/gl.hpp>
#include <heatsink/traits/enum.hpp>

namespace heatsink::gl {
	/**
	 * A static namespace that provides utility functions specific to OpenGL
	 * enumerations related to objects/names. This class (and the related traits
	 * classes) work like the traits objects from the standard library, but
	 * take an enum value rather than a type parameter.
	 */
	template<GLenum V> requires (is_standard_object(V))
	class name_traits {
	public:
		/**
		 * Determine whether the `GLenum` value (an object type) has a valid
		 * default identifier (that is, name `0`). This includes framebuffers
		 * (for the screen buffer) and textures (for unbinding).
		 */
		static constexpr bool is_default_constructible = false;
		/**
		 * Determine whether the `GLenum` value (an object type) can be bound to
		 * multiple targets in an OpenGL context. This means that the
		 * `glBind*()` command will have at least a `GLenum target` argument.
		 */
		static constexpr bool has_target = false;
		/**
		 * Determine whether the `GLenum` value (an object type) uses an image
		 * unit during binding. This means that the `glBind*()` command will
		 * have at least a `GLuint unit` argument. Note that a`GL_TEXTURE` is
		 * also considered an image target, although it uses `glActiveTexture`.
		 */
		static constexpr bool has_image_unit = false;

	public:
		/**
		 * Create a new instance of an OpenGL object state, as specified by the
		 * type template parameter.
		 */
		static GLuint create();
		/**
		 * Destroy an existing instance of an OpenGL object state. The object
		 * passed here must have been created with the same object enumeration
		 * that is being used to destroy it.
		 */
		static void destroy(GLuint);

		/**
		 * Perform a `glBind*()` call based on the object enumeration specified
		 * in the type. This will require a mixture of object name, bind target,
		 * and bind unit, depending on the `variable` and `image` parameters of
		 * this target. Note that the order is always `name, target, unit`.
		 */
		static void bind(...);

	private:
		// Prevent a `name_traits` object from being constructed.
		name_traits() = default;
	};
}
