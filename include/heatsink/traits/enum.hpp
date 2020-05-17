#pragma once

#include <heatsink/platform/gl.hpp>

namespace heatsink::gl {
	/**
	 * Determine whether the `GLenum` value (an enum type) represents an OpenGL
	 * object type. Note that this includes all objects, including programs,
	 * shaders, and sync objects.
	 * 
	 * FIXME: determine the best enum value for sync objects.
	 */
	constexpr bool is_object(GLenum);
	/**
	 * Determine whether the `GLenum` value (an enum type) represents an OpenGL
	 * "standard" object type. Note that this does not include objects that do
	 * not follow normal conventions (programs, shaders, etc).
	 */
	constexpr bool is_standard_object(GLenum);
}

namespace heatsink::gl {
	constexpr bool is_object(GLenum e) {
		switch (e) {
			case GL_BUFFER:
			case GL_FRAMEBUFFER:
			case GL_PROGRAM_PIPELINE:
			case GL_QUERY:
			case GL_RENDERBUFFER:
			case GL_SAMPLER:
			case GL_TEXTURE:
			case GL_TRANSFORM_FEEDBACK:
			case GL_VERTEX_ARRAY:
			case GL_SHADER:
			case GL_PROGRAM:
			    return true;

			default: return false;
		}
	}

	constexpr bool is_standard_object(GLenum e) {
		switch (e) {
			case GL_BUFFER:
			case GL_FRAMEBUFFER:
			case GL_PROGRAM_PIPELINE:
			case GL_QUERY:
			case GL_RENDERBUFFER:
			case GL_SAMPLER:
			case GL_TEXTURE:
			case GL_TRANSFORM_FEEDBACK:
			case GL_VERTEX_ARRAY:
			    return true;

			default: return false;
		}
	}
}
