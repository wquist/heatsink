#include <heatsink/platform/context.hpp>

#include <iostream>

#include <glfw/glfw3.h>

#include <heatsink/error/exception.hpp>
#include <heatsink/platform/window.hpp>

namespace {
	using context   = heatsink::context;
	using exception = heatsink::exception;

	bool g_initialized = false;

	context::profile to_profile(int glfw, bool strict) {
		switch (glfw) {
			case GLFW_OPENGL_CORE_PROFILE:
				return (strict) ? context::profile::strict : context::profile::core;
			case GLFW_OPENGL_COMPAT_PROFILE:
				return context::profile::compatibility;
			case GLFW_OPENGL_ANY_PROFILE:
				return context::profile::any;

			default:
				std::cerr << "'" << glfw << "' is not a GLFW profile enumeration value." << std::endl;
				throw exception("context::profile", "unknown GLFW profile enumeration.");
		}
	}
}

namespace heatsink {
	context::context(version v, profile p, bool debug)
	: m_handle{nullptr}, m_version{v}, m_profile{p}, m_debug{debug} {
		if (g_initialized)
			return;

		if (glfwInit() != GLFW_TRUE)
			throw exception("context", "could not initialize GLFW.");

		g_initialized = true;
	}

	context::context(const window& w)
	: m_handle{w.get()} {
		auto wh = (GLFWwindow*)m_handle;

		m_version.major = glfwGetWindowAttrib(wh, GLFW_CONTEXT_VERSION_MAJOR);
		m_version.minor = glfwGetWindowAttrib(wh, GLFW_CONTEXT_VERSION_MINOR);

		auto p = glfwGetWindowAttrib(wh, GLFW_OPENGL_PROFILE);
		auto s = glfwGetWindowAttrib(wh, GLFW_OPENGL_FORWARD_COMPAT);
		m_profile = to_profile(p, s);

		m_debug = glfwGetWindowAttrib(wh, GLFW_OPENGL_DEBUG_CONTEXT);
	}

	context::context(context&& other) noexcept
	: m_version{other.m_version}, m_profile{other.m_profile}, m_debug{other.m_debug}, m_handle{other.m_handle} {}

	context::~context() {}

	context& context::operator =(context&& other) {
		m_version = other.m_version;
		m_profile = other.m_profile;
		m_debug   = other.m_debug;

		m_handle = other.m_handle;
		return *this;
	}

	void* context::get() const {
		return m_handle;
	}

	context::version context::get_version() const {
		return m_version;
	}

	context::profile context::get_profile() const {
		return m_profile;
	}

	bool context::is_debug() const {
		return m_debug;
	}
}
