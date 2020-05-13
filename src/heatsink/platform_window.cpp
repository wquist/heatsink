#include <heatsink/platform/window.hpp>

#include <cassert>
#include <iostream>
#include <tuple>

#include <glfw/glfw3.h>

#include <heatsink/error/debug.hpp>
#include <heatsink/error/exception.hpp>
#include <heatsink/platform/gl.hpp>

namespace {
	using context   = heatsink::context;
	using exception = heatsink::exception;

	std::pair<int, bool> from_profile(context::profile p) {
		switch (p) {
			case context::profile::core:
				return std::make_pair(GLFW_OPENGL_CORE_PROFILE, false);
			case context::profile::strict:
				return std::make_pair(GLFW_OPENGL_CORE_PROFILE, true);
			case context::profile::compatibility:
				return std::make_pair(GLFW_OPENGL_COMPAT_PROFILE, false);
			case context::profile::any:
				return std::make_pair(GLFW_OPENGL_ANY_PROFILE, false);

			default:
				throw exception("context::profile", "invalid profile enumeration value.");
		}
	}
}

namespace heatsink {
	struct window::callbacks {
		static void resize(GLFWwindow* wh, int, int) {
			auto& w = *(window*)glfwGetWindowUserPointer(wh);
			int width, height;

			glfwGetWindowSize(wh, &width, &height);
			w.m_extents = glm::uvec2(width, height);

			glfwGetFramebufferSize(wh, &width, &height);
			w.m_framebuffer_extents = glm::uvec2(width, height);
		}

		static void message(GLenum, GLenum type, GLuint, GLenum sev, GLsizei, const GLchar* msg, const GLvoid*) {
			auto ts  = heatsink::gl::to_string(type);
			auto ss  = heatsink::gl::to_string(sev);
			auto out = ts + "(" + ss + ") - " + msg;

			if (type == GL_DEBUG_TYPE_ERROR || sev == GL_DEBUG_SEVERITY_HIGH)
				throw exception("platform", out);
			else
				std::cerr << "[heatsink::platform] " << out << std::endl;
		}
	};
}

namespace heatsink {
	window window::null() {
		return window(nullptr);
	}

	window window::offscreen(const context& c) {
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

		// Even though the screen buffer is unusable, a valid size must be used.
		window result(c, "", {640,480}, false);

		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		return result;
	}

	window::window(const context& c, const std::string& name, extents e, bool resize) {
		// FIXME: some sizes may be too small for the WM; try to catch this.
		assert(e.x && e.y);
		
		// Set up context-related parameters.
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, (int)c.get_version().major);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, (int)c.get_version().minor);

		auto [p, s] = from_profile(c.get_profile());
		glfwWindowHint(GLFW_OPENGL_PROFILE, p);

		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, (s) ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, (c.is_debug()) ? GLFW_TRUE : GLFW_FALSE);

		// Set up window-related parameters.
		glfwWindowHint(GLFW_RESIZABLE, (resize) ? GLFW_TRUE : GLFW_FALSE);

		m_handle = glfwCreateWindow(e.x, e.y, name.c_str(), nullptr, (GLFWwindow*)c.get());
		if (!m_handle)
			throw exception("window", "could not create GLFW window.");

		auto* wh = (GLFWwindow*)m_handle;
		// Enable class references within callbacks. Note that this pointer
		// must be updated every time the window is move assigned/constructed.
		glfwSetWindowUserPointer(wh, (void*)this);

		glfwSetWindowSizeCallback(wh, (GLFWwindowsizefun)callbacks::resize);
		// The resize callback does not use the parameter values, so trigger it
		// one to get the initial window/framebuffer size.
		callbacks::resize(wh, -1, -1);

		this->use();
		// The OpenGL debug callback is only available in versions >=4.3
		if (c.is_debug() && c.get_version() >= context::version{4,3}) {
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback((GLDEBUGPROC)callbacks::message, nullptr);
		}
	}

	window::window(window&& other) noexcept
	: m_handle{other.m_handle}, m_extents{other.m_extents}, m_framebuffer_extents{other.m_framebuffer_extents} {
		glfwSetWindowUserPointer((GLFWwindow*)m_handle, (void*)this);
		other.m_handle = nullptr;
	}

	window::~window() {
		if (m_handle)
			glfwDestroyWindow((GLFWwindow*)m_handle);
	}

	window& window::operator =(window&& other) {
		if (m_handle)
			glfwDestroyWindow((GLFWwindow*)m_handle);

		m_handle              = other.m_handle;
		m_extents             = other.m_extents;
		m_framebuffer_extents = other.m_framebuffer_extents;

		glfwSetWindowUserPointer((GLFWwindow*)m_handle, (void*)this);
		other.m_handle = nullptr;

		return *this;
	}

	window::window(std::nullptr_t)
	: m_handle{nullptr} {}

	void window::use() const {
		assert(m_handle);
		glfwMakeContextCurrent((GLFWwindow*)m_handle);
	}

	bool window::flush_buffers() const {
		assert(m_handle);
		auto* wh = (GLFWwindow*)m_handle;

		glfwSwapBuffers(wh);
		// Process events for all windows; this may invoke callbacks of any
		// windows currently in use, not just the one being refreshed.
		glfwPollEvents();

		return !glfwWindowShouldClose(wh);
	}

	void window::flush_errors() const {
		// `glGetError()` acts like a stack; multiple errors can be queued, the
		// end signified by a `GL_NO_ERROR` return value.
		for (GLenum e; (e = glGetError()) != GL_NO_ERROR;) {
			auto msg = gl::to_string(e);
			callbacks::message(0, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH, 0, msg.c_str(), nullptr);
		}
	}

	bool window::is_valid() const {
		return (m_handle != nullptr);
	}

	void* window::get() const {
		assert(m_handle);
		return m_handle;
	}

	window::extents window::get_extents() const {
		return m_extents;
	}

	window::extents window::get_framebuffer_extents() const {
		return m_framebuffer_extents;
	}
}
