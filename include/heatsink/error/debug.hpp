#pragma once

#include <string>

#include <heatsink/platform/gl.hpp>

namespace heatsink::gl {
	/**
	 * Format a debug/error enumeration to a string description equivalent. If
	 * none is defined, a string `"[GL:XXX]"` will be returned, where `XXX` is
	 * the integer value of the enumeration. Note that the descriptions do not
	 * include their context; for example, `GL_FRAMEBUFFER_UNSUPPORTED` does not
	 * mention the word "framebuffer" in its description.
	 */
	std::string to_string(GLenum);
}
