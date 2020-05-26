#pragma once

#include <cstdlib>
#include <tuple>

#include <heatsink/platform/gl.hpp>

namespace heatsink::gl {
	/**
	 * A static namespace that provides utility functions specific to OpenGL
	 * enumerations related to image formats. This class (and the related traits
	 * classes) work like the traits objects from the standard library; however,
	 * because they work on enums instead of types, they do not take a template
	 * parameter and instead take an argument directly. This allows them to work
	 * during runtime as well (which is more common in this library).
	 */
	class format_traits {
	public:
		/**
		 * Check if the given image format is "sized". This means that the enum,
		 * in addition to the components, also specifies a size and type. For
		 * example, `GL_RGB8` is an 8 bits per channel UNORM format.
		 */
		static constexpr bool is_sized(GLenum);
		/**
		 * Calculate the base (unsized) type of an OpenGL image format. This is
		 * the type that would be used as the "pixel format" when loading a
		 * texture with data (e.g., `GL_RGB` is the base type of `GL_RGB8`).
		 */
		static constexpr GLenum remove_size(GLenum);

		/**
		 * Calculate the most appropriate type enum for an OpenGL image format.
		 * This is the "pixel type" of the data used to fill a texture. For
		 * example, the most appropriate type for `GL_RGB16` is
		 * `GL_UNSIGNED_SHORT` (even though most enum types can be used).
		 */
		static constexpr GLenum underlying_datatype(GLenum);

		/**
		 * Calculate the format "extent", or number of color channels in an
		 * OpenGL image format. For example, `GL_RGB` has `3` channels. Note
		 * that depth and stencil formats are considered to have a single
		 * channel, and depth-stencil formats to have two channels.
		 */
		static constexpr std::size_t extent(GLenum);
		/**
		 * Calculate the reversed format for the given enumeration. Note that
		 * this method also takes a datatype parameter; in some cases, a
		 * specific format may instead be reversible by using a `_REV` datatype
		 * (leaving the actual format as-is). This takes priority over any
		 * format transformation that would normally take place. Also note that
		 * the result is split into format and datatype, as all reversed formats
		 * are unsized.
		 */
		static constexpr std::pair<GLenum, GLenum> reverse(GLenum);

	private:
		// Prevent a `format_traits` object from being constructed.
		format_traits() = default;
	};
}

namespace heatsink::gl {
	constexpr bool format_traits::is_sized(GLenum e) {
		return (underlying_datatype(e) != GL_NONE);
	}

	constexpr GLenum format_traits::remove_size(GLenum e) {
		switch (e) {
			case GL_RED:
			case GL_R8:
			case GL_R8_SNORM:
			case GL_R16:
			case GL_R16_SNORM:
			case GL_R16F:
			case GL_R32F:
				return GL_RED;
			case GL_RED_INTEGER:
			case GL_R8I:
			case GL_R8UI:
			case GL_R16I:
			case GL_R16UI:
			case GL_R32I:
			case GL_R32UI:
				return GL_RED_INTEGER;

			case GL_RG:
			case GL_RG8:
			case GL_RG8_SNORM:
			case GL_RG16:
			case GL_RG16_SNORM:
			case GL_RG16F:
			case GL_RG32F:
				return GL_RG;
			case GL_RG_INTEGER:
			case GL_RG8I:
			case GL_RG8UI:
			case GL_RG16I:
			case GL_RG16UI:
			case GL_RG32I:
			case GL_RG32UI:
				return GL_RG_INTEGER;

			case GL_RGB:
			case GL_RGB4:
			case GL_RGB5:
			case GL_RGB8:
			case GL_RGB8_SNORM:
			case GL_RGB10:
			case GL_RGB12:
			case GL_RGB16:
			case GL_RGB16_SNORM:
			case GL_RGB16F:
			case GL_RGB32F:
				return GL_RGB;
			case GL_RGB_INTEGER:
			case GL_RGB8I:
			case GL_RGB8UI:
			case GL_RGB16I:
			case GL_RGB16UI:
			case GL_RGB32I:
			case GL_RGB32UI:
				return GL_RGB_INTEGER;

			case GL_RGBA:
			case GL_RGBA2:
			case GL_RGBA4:
			case GL_RGBA8:
			case GL_RGBA8_SNORM:
			case GL_RGBA12:
			case GL_RGBA16:
			case GL_RGBA16_SNORM:
			case GL_RGBA16F:
			case GL_RGBA32F:
				return GL_RGBA;
			case GL_RGBA_INTEGER:
			case GL_RGBA8I:
			case GL_RGBA8UI:
			case GL_RGBA16I:
			case GL_RGBA16UI:
			case GL_RGBA32I:
			case GL_RGBA32UI:
				return GL_RGBA_INTEGER;

			case GL_DEPTH_COMPONENT:
			case GL_DEPTH_COMPONENT16:
			case GL_DEPTH_COMPONENT24:
			case GL_DEPTH_COMPONENT32:
			case GL_DEPTH_COMPONENT32F:
				return GL_DEPTH_COMPONENT;

			case GL_STENCIL:
			case GL_STENCIL_INDEX8:
				return GL_STENCIL;

			case GL_DEPTH_STENCIL:
			case GL_DEPTH24_STENCIL8:
			case GL_DEPTH32F_STENCIL8:
				return GL_DEPTH_STENCIL;

			case GL_BGR:             return GL_BGR;
			case GL_BGR_INTEGER:     return GL_BGR_INTEGER;
			case GL_BGRA:            return GL_BGRA;
			case GL_BGRA_INTEGER:    return GL_BGRA_INTEGER;

			case GL_R3_G3_B2:        return GL_RGB;
			case GL_RGB5_A1:         return GL_RGBA;
			case GL_RGB10_A2:        return GL_RGBA;
			case GL_RGB10_A2UI:      return GL_RGBA_INTEGER;
			case GL_R11F_G11F_B10F:  return GL_RGB;
			case GL_RGB9_E5:         return GL_RGB;
			case GL_RGB565:          return GL_RGB;

			case GL_SRGB:            return GL_RGB;
			case GL_SRGB8:           return GL_RGB;
			case GL_SRGB_ALPHA:      return GL_RGBA;
			case GL_SRGB8_ALPHA8:    return GL_RGBA;

			default: return GL_NONE;
		}
	}

	constexpr GLenum format_traits::underlying_datatype(GLenum e) {
		switch (e) {
			case GL_R8:
			case GL_R8UI:
			case GL_RG8:
			case GL_RG8UI:
			case GL_RGB4:
			case GL_RGB5:
			case GL_RGB8:
			case GL_RGB8UI:
			case GL_RGBA4:
			case GL_RGBA8:
			case GL_RGBA8UI:
			case GL_STENCIL_INDEX8:
				return GL_UNSIGNED_BYTE;
			case GL_R8_SNORM:
			case GL_R8I:
			case GL_RG8_SNORM:
			case GL_RG8I:
			case GL_RGB8_SNORM:
			case GL_RGB8I:
			case GL_RGBA8_SNORM:
			case GL_RGBA8I:
				return GL_BYTE;

			case GL_R16:
			case GL_R16UI:
			case GL_RG16:
			case GL_RG16UI:
			case GL_RGB10:
			case GL_RGB12:
			case GL_RGB16:
			case GL_RGB16UI:
			case GL_RGBA12:
			case GL_RGBA16:
			case GL_RGBA16UI:
			case GL_DEPTH_COMPONENT16:
				return GL_UNSIGNED_SHORT;
			case GL_R16_SNORM:
			case GL_R16I:
			case GL_RG16_SNORM:
			case GL_RG16I:
			case GL_RGB16_SNORM:
			case GL_RGB16I:
			case GL_RGBA16_SNORM:
			case GL_RGBA16I:
				return GL_SHORT;
			
			case GL_R32UI:
			case GL_RG32UI:
			case GL_RGB32UI:
			case GL_RGBA32UI:
			case GL_DEPTH_COMPONENT24:
			case GL_DEPTH_COMPONENT32:
				return GL_UNSIGNED_INT;
			case GL_R32I:
			case GL_RG32I:
			case GL_RGB32I:
			case GL_RGBA32I:
				return GL_INT;

			case GL_R16F:
			case GL_RG16F:
			case GL_RGB16F:
			case GL_RGBA16F:
				return GL_HALF_FLOAT;
			case GL_R32F:
			case GL_RG32F:
			case GL_RGB32F:
			case GL_RGBA32F:
			case GL_DEPTH_COMPONENT32F:
				return GL_FLOAT;

			case GL_R3_G3_B2:          return GL_UNSIGNED_BYTE_3_3_2;
			case GL_RGB5_A1:           return GL_UNSIGNED_SHORT_5_5_5_1;
			case GL_RGB10_A2:          return GL_UNSIGNED_INT_10_10_10_2;
			case GL_RGB10_A2UI:        return GL_UNSIGNED_INT_10_10_10_2;
			case GL_R11F_G11F_B10F:    return GL_UNSIGNED_INT_10F_11F_11F_REV;
			case GL_RGB9_E5:           return GL_UNSIGNED_INT_5_9_9_9_REV;
			case GL_RGB565:            return GL_UNSIGNED_SHORT_5_6_5;

			case GL_SRGB8:             return GL_UNSIGNED_BYTE;
			case GL_SRGB8_ALPHA8:      return GL_UNSIGNED_BYTE;

			case GL_DEPTH24_STENCIL8:  return GL_UNSIGNED_INT_24_8;
			case GL_DEPTH32F_STENCIL8: return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;

			default: return GL_NONE;
		}
	}

	constexpr std::size_t format_traits::extent(GLenum e) {
		// Handle all image formats by first converting to their unsized form.
		e = remove_size(e);

		switch (e) {
			case GL_RED:
			case GL_RED_INTEGER:
			case GL_DEPTH_COMPONENT:
			case GL_STENCIL:
				return 1;

			case GL_RG:
			case GL_RG_INTEGER:
			case GL_DEPTH_STENCIL:
				return 2;

			case GL_RGB:
			case GL_RGB_INTEGER:
				return 3;

			case GL_RGBA:
			case GL_RGBA_INTEGER:
				return 4; 

			default: return 0;
		}
	}

	constexpr std::pair<GLenum, GLenum> format_traits::reverse(GLenum e) {
		auto f = remove_size(e);
		auto t = underlying_datatype(e);

		GLenum datatype = GL_NONE;
		switch (t) {
			case GL_UNSIGNED_BYTE_3_3_2:         datatype = GL_UNSIGNED_BYTE_2_3_3_REV;     break;
			case GL_UNSIGNED_SHORT_5_6_5:        datatype = GL_UNSIGNED_SHORT_5_6_5_REV;    break;
			case GL_UNSIGNED_SHORT_4_4_4_4:      datatype = GL_UNSIGNED_SHORT_4_4_4_4_REV;  break;
			case GL_UNSIGNED_SHORT_5_5_5_1:      datatype = GL_UNSIGNED_SHORT_1_5_5_5_REV;  break;
			case GL_UNSIGNED_INT_8_8_8_8:        datatype = GL_UNSIGNED_INT_8_8_8_8_REV;    break;
			case GL_UNSIGNED_INT_10_10_10_2:     datatype = GL_UNSIGNED_INT_2_10_10_10_REV; break;

			case GL_UNSIGNED_BYTE_2_3_3_REV:     datatype = GL_UNSIGNED_BYTE_3_3_2;         break;
			case GL_UNSIGNED_SHORT_5_6_5_REV:    datatype = GL_UNSIGNED_SHORT_5_6_5;        break;
			case GL_UNSIGNED_SHORT_4_4_4_4_REV:  datatype = GL_UNSIGNED_SHORT_4_4_4_4;      break;
			case GL_UNSIGNED_SHORT_1_5_5_5_REV:  datatype = GL_UNSIGNED_SHORT_5_5_5_1;      break;
			case GL_UNSIGNED_INT_8_8_8_8_REV:    datatype = GL_UNSIGNED_INT_8_8_8_8;        break;
			case GL_UNSIGNED_INT_2_10_10_10_REV: datatype = GL_UNSIGNED_INT_10_10_10_2;     break;
		}

		if (datatype != GL_NONE)
			return std::pair<GLenum, GLenum>(f, datatype);

		GLenum format = GL_NONE;
		switch (f) {
			case GL_RGB:          format = GL_BGR;          break;
			case GL_RGB_INTEGER:  format = GL_BGR_INTEGER;  break;
			case GL_RGBA:         format = GL_BGRA;         break;
			case GL_RGBA_INTEGER: format = GL_BGRA_INTEGER; break;

			case GL_BGR:          format = GL_RGB;          break;
			case GL_BGR_INTEGER:  format = GL_RGB_INTEGER;  break;
			case GL_BGRA:         format = GL_RGBA;         break;
			case GL_BGRA_INTEGER: format = GL_RGBA_INTEGER; break;
		}

		if (format != GL_NONE)
			return std::pair<GLenum, GLenum>(format, t);
		else
			return std::pair<GLenum, GLenum>(GL_NONE, GL_NONE);
	}
}
