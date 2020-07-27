#pragma once

#include <string_view>

#include <heatsink/platform/gl.hpp>

namespace heatsink::gl {
	/**
	 * A static namespace that provides utility functions specific to OpenGL
	 * enumerations related to shaders/programs, as well as uniforms and
	 * attributes. This class (and the related traits classes) work like the
	 * traits objects from the standard library; however, because they work on
	 * enums instead of types, they do not take a template parameter and instead
	 * take an argument directly.
	 */
	class shader_traits {
	public:
		/**
		 * Determine whether the given `GLenum` uniform type is an "opaque"
		 * type. This means that it represents an object type that is not
		 * directly settable through the uniform interface. Rather, it is set
		 * through an integer identifier (see samplers, images, etc.).
		 */
		static constexpr bool is_opaque(GLenum);

		/**
		 * Determine whether a uniform type (specified by its `GLenum` value)
		 * can be assigned by the `glUniform*()` call for the type of `src` (for
		 * example, `GL_INT` is not assignable through `glUniform1f()`). This is
		 * true for all types where `src==dest`, and also for booleans when
		 * `src` is an integer, unsigned, or floating point of the same rank.
		 */
		static constexpr bool is_assignable(GLenum dest, GLenum src);

		/**
		 * Check if the given attribute name is one of the built-in GLSL shader
		 * variables. The OpenGL attribute introspection method includes these
		 * in the list of active attributes, but they do not have locations.
		 */
		static constexpr bool is_builtin(std::string_view name);

	private:
		// Prevent a `shader_traits` object from being constructed.
		shader_traits() = default;
	};
}

namespace heatsink::gl {
	constexpr bool shader_traits::is_opaque(GLenum e) {
		switch (e) {
			case GL_SAMPLER:
			case GL_SAMPLER_1D:
			case GL_SAMPLER_1D_ARRAY:
			case GL_SAMPLER_1D_SHADOW:
			case GL_SAMPLER_1D_ARRAY_SHADOW:
			case GL_SAMPLER_2D:
			case GL_SAMPLER_2D_ARRAY:
			case GL_SAMPLER_2D_SHADOW:
			case GL_SAMPLER_2D_ARRAY_SHADOW:
			case GL_SAMPLER_2D_MULTISAMPLE:
			case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
			case GL_SAMPLER_2D_RECT:
			case GL_SAMPLER_2D_RECT_SHADOW:
			case GL_SAMPLER_3D:
			case GL_SAMPLER_CUBE:
			case GL_SAMPLER_CUBE_MAP_ARRAY:
			case GL_SAMPLER_CUBE_SHADOW:
			case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
			case GL_SAMPLER_BUFFER:
				return true;

			case GL_INT_SAMPLER_1D:
			case GL_INT_SAMPLER_1D_ARRAY:
			case GL_INT_SAMPLER_2D:
			case GL_INT_SAMPLER_2D_ARRAY:
			case GL_INT_SAMPLER_2D_MULTISAMPLE:
			case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
			case GL_INT_SAMPLER_2D_RECT:
			case GL_INT_SAMPLER_3D:
			case GL_INT_SAMPLER_CUBE:
			case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
			case GL_INT_SAMPLER_BUFFER:
				return true;

			case GL_UNSIGNED_INT_SAMPLER_1D:
			case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
			case GL_UNSIGNED_INT_SAMPLER_2D:
			case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
			case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
			case GL_UNSIGNED_INT_SAMPLER_3D:
			case GL_UNSIGNED_INT_SAMPLER_CUBE:
			case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
			case GL_UNSIGNED_INT_SAMPLER_BUFFER:
				return true;

			default: return false;
		}
	}

	constexpr bool shader_traits::is_assignable(GLenum dest, GLenum src) {
		if (dest == src)
			return true;
		if (is_opaque(dest) && src == GL_INT)
			return true;

		switch (dest) {
			case GL_BOOL:
				switch (src) {
					case GL_INT:
					case GL_UNSIGNED_INT:
					case GL_FLOAT:
						return true;

					default: return false;
				}
			case GL_BOOL_VEC2:
				switch (src) {
					case GL_INT_VEC2:
					case GL_UNSIGNED_INT_VEC2:
					case GL_FLOAT_VEC2:
						return true;

					default: return false;
				}
			case GL_BOOL_VEC3:
				switch (src) {
					case GL_INT_VEC3:
					case GL_UNSIGNED_INT_VEC3:
					case GL_FLOAT_VEC3:
						return true;

					default: return false;
				}
			case GL_BOOL_VEC4:
				switch (src) {
					case GL_INT_VEC4:
					case GL_UNSIGNED_INT_VEC4:
					case GL_FLOAT_VEC4:
						return true;

					default: return false;
				}

			default: return false;
		}
	}

	constexpr bool shader_traits::is_builtin(std::string_view attr_name) {
		using namespace std::string_view_literals;

		if      (attr_name == "gl_VertexID"sv)     return true;
		else if (attr_name == "gl_InstanceID"sv)   return true;
		else if (attr_name == "gl_DrawID"sv)       return true;
		else if (attr_name == "gl_BaseVertex"sv)   return true;
		else if (attr_name == "gl_BaseInstance"sv) return true;
		else return false;
	}
}
