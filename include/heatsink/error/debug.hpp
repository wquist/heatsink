#pragma once

#include <string>
#include <ostream>

#include <heatsink/platform/gl.hpp>

namespace heatsink {
	/**
	 * Retrieve a stream to write additional error information to. Many heatsink
	 * exceptions will also to an error stream; this method prepare that stream
	 * and appends a prefix ("heatsink::namespace") as exceptions do. By
	 * default, this stream points to `std::cerr`.
	 */
	std::ostream& make_error_stream(const std::string& where);

	namespace gl {
		/**
		 * Format a debug/error enumeration to a string description equivalent. If
		 * none is defined, a string `"[GL:XXX]"` will be returned, where `XXX` is
		 * the integer value of the enumeration. Note that the descriptions do not
		 * include their context; for example, `GL_FRAMEBUFFER_UNSUPPORTED` does not
		 * mention the word "framebuffer" in its description.
		 */
		std::string to_string(GLenum);
	}
}
