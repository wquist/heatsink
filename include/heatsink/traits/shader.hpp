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
		 * Check if the given attribute name is one of the built-in GLSL shader
		 * variables. The OpenGL attribute introspection method includes these
		 * in the list of active attributes, but they do not have locations.
		 */
		static constexpr bool is_builtin(std::string_view name);

	private:
		// Prevent a `shader_traits` object from being constructed.
		shader_traits() = default;
	};

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
