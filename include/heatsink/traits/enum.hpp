#pragma once

#include <type_traits>

#include <heatsink/platform/gl.hpp>

namespace heatsink::gl {
	namespace detail {
		// A `std::bool_constant`-like alias for `GLenum` constant values.
		template<GLenum V>
		using enum_constant = std::integral_constant<GLenum, V>;
	}

	/**
	 * The OpenGL enumeration equivalent of the given primitive C type (for
	 * example, `GLfloat` gives the constant `GL_FLOAT`). This also works for
	 * array types, to retrieve the `VEC`/`MAT` constants. If the type has no
	 * equivalent, this value will equal `GL_NONE`. Note that the `GLboolean`
	 * type requires special handling; it will result in `GL_UNSIGNED_BYTE`, as
	 * it is equivalent to `unsigned char`. To retrieve the `GL_BOOL*`
	 * constants, use the built-in `bool` type.
	 */
	template<class>
	struct make_enum : detail::enum_constant<GL_NONE> {};
	template<class T>
	constexpr auto make_enum_v = make_enum<T>::value;

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

	/**
	 * Calculate an extent of the given OpenGL enum type. Scalar types have an
	 * extent of 0, while array types have an extent of their array size at
	 * the given dimension (N). N is always `0` or `1` for OpenGL types.
	 */
	constexpr std::size_t extent(GLenum, std::size_t n = 0);
	/**
	 * Retrieve the base type of the given OpenGL enum type. This is equivalent
	 * to the type without any array dimensions; for example, `GL_FLOAT_MAT4x2`
	 * has a base type of `GL_FLOAT`. Non-array types are returned unchanged.
	 */
	constexpr GLenum remove_all_extents(GLenum);
}

namespace heatsink::gl {
	template<> struct make_enum<bool>           : detail::enum_constant<GL_BOOL> {};
	template<> struct make_enum<GLbyte>         : detail::enum_constant<GL_BYTE> {};
	template<> struct make_enum<GLshort>        : detail::enum_constant<GL_SHORT> {};
	template<> struct make_enum<GLint>          : detail::enum_constant<GL_INT> {};
	template<> struct make_enum<GLubyte>        : detail::enum_constant<GL_UNSIGNED_BYTE> {};
	template<> struct make_enum<GLushort>       : detail::enum_constant<GL_UNSIGNED_SHORT> {};
	template<> struct make_enum<GLuint>         : detail::enum_constant<GL_UNSIGNED_INT> {};
	template<> struct make_enum<GLfloat>        : detail::enum_constant<GL_FLOAT> {};
	template<> struct make_enum<GLdouble>       : detail::enum_constant<GL_DOUBLE> {};
	template<> struct make_enum<bool[2]>        : detail::enum_constant<GL_BOOL_VEC2> {};
	template<> struct make_enum<bool[3]>        : detail::enum_constant<GL_BOOL_VEC3> {};
	template<> struct make_enum<bool[4]>        : detail::enum_constant<GL_BOOL_VEC4> {};
	template<> struct make_enum<GLint[2]>       : detail::enum_constant<GL_INT_VEC2> {};
	template<> struct make_enum<GLint[3]>       : detail::enum_constant<GL_INT_VEC3> {};
	template<> struct make_enum<GLint[4]>       : detail::enum_constant<GL_INT_VEC4> {};
	template<> struct make_enum<GLuint[2]>      : detail::enum_constant<GL_UNSIGNED_INT_VEC2> {};
	template<> struct make_enum<GLuint[3]>      : detail::enum_constant<GL_UNSIGNED_INT_VEC3> {};
	template<> struct make_enum<GLuint[4]>      : detail::enum_constant<GL_UNSIGNED_INT_VEC4> {};
	template<> struct make_enum<GLfloat[2]>     : detail::enum_constant<GL_FLOAT_VEC2> {};
	template<> struct make_enum<GLfloat[3]>     : detail::enum_constant<GL_FLOAT_VEC3> {};
	template<> struct make_enum<GLfloat[4]>     : detail::enum_constant<GL_FLOAT_VEC4> {};
	template<> struct make_enum<GLdouble[2]>    : detail::enum_constant<GL_DOUBLE_VEC2> {};
	template<> struct make_enum<GLdouble[3]>    : detail::enum_constant<GL_DOUBLE_VEC3> {};
	template<> struct make_enum<GLdouble[4]>    : detail::enum_constant<GL_DOUBLE_VEC4> {};
	template<> struct make_enum<GLfloat[2][2]>  : detail::enum_constant<GL_FLOAT_MAT2> {};
	template<> struct make_enum<GLfloat[2][3]>  : detail::enum_constant<GL_FLOAT_MAT2x3> {};
	template<> struct make_enum<GLfloat[2][4]>  : detail::enum_constant<GL_FLOAT_MAT2x4> {};
	template<> struct make_enum<GLfloat[3][2]>  : detail::enum_constant<GL_FLOAT_MAT3x2> {};
	template<> struct make_enum<GLfloat[3][3]>  : detail::enum_constant<GL_FLOAT_MAT3> {};
	template<> struct make_enum<GLfloat[3][4]>  : detail::enum_constant<GL_FLOAT_MAT3x4> {};
	template<> struct make_enum<GLfloat[4][2]>  : detail::enum_constant<GL_FLOAT_MAT4x2> {};
	template<> struct make_enum<GLfloat[4][3]>  : detail::enum_constant<GL_FLOAT_MAT4x3> {};
	template<> struct make_enum<GLfloat[4][4]>  : detail::enum_constant<GL_FLOAT_MAT4> {};
	template<> struct make_enum<GLdouble[2][2]> : detail::enum_constant<GL_DOUBLE_MAT2> {};
	template<> struct make_enum<GLdouble[2][3]> : detail::enum_constant<GL_DOUBLE_MAT2x3> {};
	template<> struct make_enum<GLdouble[2][4]> : detail::enum_constant<GL_DOUBLE_MAT2x4> {};
	template<> struct make_enum<GLdouble[3][2]> : detail::enum_constant<GL_DOUBLE_MAT3x2> {};
	template<> struct make_enum<GLdouble[3][3]> : detail::enum_constant<GL_DOUBLE_MAT3> {};
	template<> struct make_enum<GLdouble[3][4]> : detail::enum_constant<GL_DOUBLE_MAT3x4> {};
	template<> struct make_enum<GLdouble[4][2]> : detail::enum_constant<GL_DOUBLE_MAT4x2> {};
	template<> struct make_enum<GLdouble[4][3]> : detail::enum_constant<GL_DOUBLE_MAT4x3> {};
	template<> struct make_enum<GLdouble[4][4]> : detail::enum_constant<GL_DOUBLE_MAT4> {};

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

	constexpr std::size_t extent(GLenum e, std::size_t n) {
		switch (e) {
			case GL_BOOL_VEC2:
			case GL_INT_VEC2:
			case GL_UNSIGNED_INT_VEC2:
			case GL_FLOAT_VEC2:
			case GL_DOUBLE_VEC2:
				return (n == 0) ? 2 : 0;

			case GL_BOOL_VEC3:
			case GL_INT_VEC3:
			case GL_UNSIGNED_INT_VEC3:
			case GL_FLOAT_VEC3:
			case GL_DOUBLE_VEC3:
				return (n == 0) ? 3 : 0;

			case GL_BOOL_VEC4:
			case GL_INT_VEC4:
			case GL_UNSIGNED_INT_VEC4:
			case GL_FLOAT_VEC4:
			case GL_DOUBLE_VEC4:
				return (n == 0) ? 4 : 0;

			case GL_FLOAT_MAT2:
			case GL_DOUBLE_MAT2:
				return (n < 2) ? 2 : 0;
			case GL_FLOAT_MAT2x3:
			case GL_DOUBLE_MAT2x3:
				return (n == 1) ? 2 : (n == 0) ? 3 : 0;
			case GL_FLOAT_MAT2x4:
			case GL_DOUBLE_MAT2x4:
				return (n == 1) ? 2 : (n == 0) ? 4 : 0;

			case GL_FLOAT_MAT3x2:
			case GL_DOUBLE_MAT3x2:
				return (n == 1) ? 3 : (n == 0) ? 2 : 0;
			case GL_FLOAT_MAT3:
			case GL_DOUBLE_MAT3:
				return (n < 2) ? 3 : 0;
			case GL_FLOAT_MAT3x4:
			case GL_DOUBLE_MAT3x4:
				return (n == 1) ? 3 : (n == 0) ? 4 : 0;
			
			case GL_FLOAT_MAT4x2:
			case GL_DOUBLE_MAT4x2:
				return (n == 1) ? 4 : (n == 0) ? 2 : 0;
			case GL_FLOAT_MAT4x3:
			case GL_DOUBLE_MAT4x3:
				return (n == 1) ? 4 : (n == 0) ? 3 : 0;
			case GL_FLOAT_MAT4:
			case GL_DOUBLE_MAT4:
				return (n < 2) ? 4 : 0;

			default: return 0;
		}
	}

	constexpr GLenum remove_all_extents(GLenum e) {
		switch (e) {
			case GL_BOOL_VEC2:
			case GL_BOOL_VEC3:
			case GL_BOOL_VEC4:
				return GL_BOOL;
			
			case GL_INT_VEC2:
			case GL_INT_VEC3:
			case GL_INT_VEC4:
				return GL_INT;
			
			case GL_UNSIGNED_INT_VEC2:
			case GL_UNSIGNED_INT_VEC3:
			case GL_UNSIGNED_INT_VEC4:
				return GL_UNSIGNED_INT;

			case GL_FLOAT_VEC2:
			case GL_FLOAT_VEC3:
			case GL_FLOAT_VEC4:
			case GL_FLOAT_MAT2:
			case GL_FLOAT_MAT2x3:
			case GL_FLOAT_MAT2x4:
			case GL_FLOAT_MAT3x2:
			case GL_FLOAT_MAT3:
			case GL_FLOAT_MAT3x4:
			case GL_FLOAT_MAT4x2:
			case GL_FLOAT_MAT4x3:
			case GL_FLOAT_MAT4:
				return GL_FLOAT;

			case GL_DOUBLE_VEC2:
			case GL_DOUBLE_VEC3:
			case GL_DOUBLE_VEC4:
			case GL_DOUBLE_MAT2:
			case GL_DOUBLE_MAT2x3:
			case GL_DOUBLE_MAT2x4:
			case GL_DOUBLE_MAT3x2:
			case GL_DOUBLE_MAT3:
			case GL_DOUBLE_MAT3x4:
			case GL_DOUBLE_MAT4x2:
			case GL_DOUBLE_MAT4x3:
			case GL_DOUBLE_MAT4:
				return GL_DOUBLE;

			default: return e;
		}
	}
}
