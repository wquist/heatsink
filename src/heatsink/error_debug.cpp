#include <heatsink/error/debug.hpp>

#include <iostream>

using namespace std::string_literals;

namespace heatsink {
	std::ostream& make_error_stream(const std::string& where) {
		return (std::cerr << "[heatsink::" << where << "] ");
	}
}

namespace heatsink::gl {
	std::string to_string(GLenum e) {
		switch (e) {
			case GL_INVALID_ENUM:                              return "invalid enum";
			case GL_INVALID_VALUE:                             return "invalid value";
			case GL_INVALID_OPERATION:                         return "invalid operation";
			case GL_STACK_OVERFLOW:                            return "stack overflow";
			case GL_STACK_UNDERFLOW:                           return "stack underflow";
			case GL_OUT_OF_MEMORY:                             return "out of memory";
			case GL_INVALID_FRAMEBUFFER_OPERATION:             return "invalid framebufer operation";
			case GL_CONTEXT_LOST:                              return "context lost";

			case GL_DEBUG_TYPE_ERROR:                          return "error";
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:            return "deprecated behavior";
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:             return "undefined behavior";
			case GL_DEBUG_TYPE_PORTABILITY:                    return "portability";
			case GL_DEBUG_TYPE_MARKER:                         return "debug marker";
			case GL_DEBUG_TYPE_PUSH_GROUP:                     return "push group";
			case GL_DEBUG_TYPE_POP_GROUP:                      return "pop group";
			case GL_DEBUG_TYPE_OTHER:                          return "other debug";

			case GL_DEBUG_SEVERITY_HIGH:                       return "high severity";
			case GL_DEBUG_SEVERITY_MEDIUM:                     return "medium severity";
			case GL_DEBUG_SEVERITY_LOW:                        return "low severity";
			case GL_DEBUG_SEVERITY_NOTIFICATION:               return "notification";

			case GL_VERTEX_SHADER:                             return "vertex shader";
			case GL_FRAGMENT_SHADER:                           return "fragment shader";
			case GL_TESS_CONTROL_SHADER:                       return "tesselation control shader";
			case GL_TESS_EVALUATION_SHADER:                    return "tesselation evaluation shader";
			case GL_GEOMETRY_SHADER:                           return "geometry shader";
			case GL_COMPUTE_SHADER:                            return "compute shader";

			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         return "incomplete attachment";
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "no attachments";
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:        return "invalid draw buffer array";
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:        return "invalid read buffer";
			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:        return "non-uniform multisample count";
			case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:      return "non-uniform layer count";
			case GL_FRAMEBUFFER_UNSUPPORTED:                   return "unsupported attachment combination";

			default: return "[GL:"s + std::to_string(e) + "]"s;
		}
	}
}
