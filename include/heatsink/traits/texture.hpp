#pragma once

#include <cstdlib>

#include <heatsink/platform/gl.hpp>

namespace heatsink::gl {
	/**
	 * A static namespace that provides utility functions specific to OpenGL
	 * enumerations related to texture modes. This class (and the related traits
	 * classes) work like the traits objects from the standard library; however,
	 * because they work on enums instead of types, they do not take a template
	 * parameter and instead take an argument directly.
	 */
	class texture_traits {
	public:
		/**
		 * Determine if an OpenGL texture target type is a multisample texture.
		 * Only `GL_TEXTURE_2D_MULTISAMPLE` and
		 * `GL_TEXTURE_2D_MULTISAMPLE_ARRAY` will return true here.
		 */
		static constexpr bool is_multisample(GLenum);
		/**
		 * Determine if an OpenGL texture target is an array texture. Array
		 * textures use texture methods at one higher rank than their actual
		 * dimensions, to represent the number of layers.
		 */
		static constexpr bool is_array(GLenum);
		/**
		 * Determine if an OpenGL texture target is a cubemap (or cubemap
		 * array). Cubemaps (but not cubemap arrays) are represented with
		 * special face targets, rather than using an layer dimension.
		 */
		static constexpr bool is_cubemap(GLenum);

		/**
		 * Calculate the rank of an OpenGL texture target type. This is
		 * equivalent the the number of "dimensions" a texture has, plus one if
		 * it is an array. Note that both cube maps and cube map arrays are 2D
		 * arrays (rank = 3).
		 */
		static constexpr std::size_t rank(GLenum);

	private:
		// Prevent a `texture_traits` object from being constructed.
		texture_traits() = default;
	}
}

namespace heatsink::gl {
	constexpr bool texture_traits::is_multisample(GLenum e) {
		switch (e) {
			case GL_TEXTURE_2D_MULTISAMPLE:
			case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
				return true;

			default: return false;
		}
	}

	constexpr bool texture_traits::is_array(GLenum e) {
		switch (e) {
			case GL_TEXTURE_1D_ARRAY:
			case GL_TEXTURE_2D_ARRAY:
			case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
			case GL_TEXTURE_CUBE_MAP_ARRAY:
				return true;

			default: return false;
		}
	}

	constexpr bool texture_traits::is_cubemap(GLenum e) {
		switch (e) {
			case GL_TEXTURE_CUBE_MAP:
			case GL_TEXTURE_CUBE_MAP_ARRAY:
				return true;

			default: return false;
		}
	}

	constexpr std::size_t texture_traits::rank(GLenum e) {
		switch (e) {
			case GL_TEXTURE_1D:
			case GL_TEXTURE_BUFFER:
				return 1;

			case GL_TEXTURE_1D_ARRAY:
			case GL_TEXTURE_2D:
			case GL_TEXTURE_2D_MULTISAMPLE:
			case GL_TEXTURE_RECTANGLE:
				return 2;

			case GL_TEXTURE_2D_ARRAY:
			case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
			case GL_TEXTURE_3D:
			case GL_TEXTURE_CUBE_MAP:
			case GL_TEXTURE_CUBE_MAP_ARRAY:
				return 3;

			default: return 0;
		}
	}
}
