#pragma once

#include <cstdlib>
#include <memory>
#include <type_traits>
#include <utility>

#include <heatsink/platform/gl.hpp>
#include <heatsink/traits/tensor.hpp>

namespace heatsink {
	/**
	 * Determine the actual type and memory address of a tensor. The result will
	 * point to the first element of the tensor and have a type equivalent to a
	 * single element of `tensor_decay<T>`.
	 */
	template<tensor T>
	constexpr const auto* address_of(T&);

	/**
	 * Calculate the byte offset of a member within a standard-layout type. This
	 * is functionally equivalent to the `offsetof()` macro, but functions on
	 * pointer-to-member values instead of taking the member name directly.
	 */
	template<class T, standard_layout Struct>
	constexpr std::size_t offset_of(T Struct::*member);

	namespace gl {
		/**
		 * Calculate the equivalent number of bytes an OpenGL data type occupies
		 * in program space. This is the same as calling `sizeof()` on the
		 * corresponding C data type.
		 */
		constexpr std::size_t size_of(GLenum);
		/**
		 * Determine whether the given OpenGL data type is "packed"; that is,
		 * whether it represents multiple values within a single program-space
		 * value (such as `GL_UNSIGNED_INT_24_8` representing two channels).
		 */
		constexpr bool is_packed(GLenum);
	}
}

namespace heatsink {
	template<tensor T>
	constexpr const auto* address_of(T& t) {
		using element_type = std::remove_all_extents<tensor_decay_t<T>>;
		return reinterpret_cast<const element_type*>(std::addressof(t));
	}

	template<class T, standard_layout Struct>
	constexpr std::size_t offset_of(T Struct::*member) {
		// Launder a size-equivalent byte array to get valid pointer values.
		// - Casting `nullptr` to access the member is undefined behavior.
		// - Constructing `Struct` directly will not work if a default
		//   constructor is not available.
		alignas(Struct) std::byte storage[sizeof(Struct)] = {};

		auto* base = std::launder((Struct*)(void*)storage);
		return (std::intptr_t)&(base->*member) - (std::intptr_t)base;
	}

	constexpr std::size_t gl::size_of(GLenum e) {
		switch (e) {
			case GL_BOOL:                           return sizeof(bool);
			case GL_BYTE:                           return sizeof(GLbyte);
			case GL_SHORT:                          return sizeof(GLshort);
			case GL_INT:                            return sizeof(GLint);
			case GL_UNSIGNED_BYTE:                  return sizeof(GLubyte);
			case GL_UNSIGNED_SHORT:                 return sizeof(GLushort);
			case GL_UNSIGNED_INT:                   return sizeof(GLuint);
			case GL_FLOAT:                          return sizeof(GLfloat);
			case GL_DOUBLE:                         return sizeof(GLdouble);

			case GL_UNSIGNED_BYTE_3_3_2:            return sizeof(GLubyte);
			case GL_UNSIGNED_BYTE_2_3_3_REV:        return sizeof(GLubyte);
			case GL_UNSIGNED_SHORT_5_6_5:           return sizeof(GLushort);
			case GL_UNSIGNED_SHORT_5_6_5_REV:       return sizeof(GLushort);
			case GL_UNSIGNED_SHORT_4_4_4_4:         return sizeof(GLushort);
			case GL_UNSIGNED_SHORT_4_4_4_4_REV:     return sizeof(GLushort);
			case GL_UNSIGNED_SHORT_5_5_5_1:         return sizeof(GLushort);
			case GL_UNSIGNED_SHORT_1_5_5_5_REV:     return sizeof(GLushort);
			case GL_UNSIGNED_INT_24_8:              return sizeof(GLuint);
			case GL_UNSIGNED_INT_10F_11F_11F_REV:   return sizeof(GLuint);
			case GL_UNSIGNED_INT_8_8_8_8:           return sizeof(GLuint);
			case GL_UNSIGNED_INT_8_8_8_8_REV:       return sizeof(GLuint);
			case GL_UNSIGNED_INT_10_10_10_2:        return sizeof(GLuint);
			case GL_UNSIGNED_INT_2_10_10_10_REV:    return sizeof(GLuint);
			case GL_UNSIGNED_INT_5_9_9_9_REV:       return sizeof(GLuint);

			case GL_FLOAT_32_UNSIGNED_INT_24_8_REV: return sizeof(GLfloat) + sizeof(GLint);

			default: return 0;
		}
	}

	constexpr bool gl::is_packed(GLenum e) {
		switch (e) {
			case GL_UNSIGNED_BYTE_3_3_2:
			case GL_UNSIGNED_BYTE_2_3_3_REV:
			case GL_UNSIGNED_SHORT_5_6_5:
			case GL_UNSIGNED_SHORT_5_6_5_REV:
			case GL_UNSIGNED_SHORT_4_4_4_4:
			case GL_UNSIGNED_SHORT_4_4_4_4_REV:
			case GL_UNSIGNED_SHORT_5_5_5_1:
			case GL_UNSIGNED_SHORT_1_5_5_5_REV:
			case GL_UNSIGNED_INT_24_8:
			case GL_UNSIGNED_INT_10F_11F_11F_REV:
			case GL_UNSIGNED_INT_8_8_8_8:
			case GL_UNSIGNED_INT_8_8_8_8_REV:
			case GL_UNSIGNED_INT_10_10_10_2:
			case GL_UNSIGNED_INT_2_10_10_10_REV:
			case GL_UNSIGNED_INT_5_9_9_9_REV:
			case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
				return true;

			default: return false;
		}
	}
}
