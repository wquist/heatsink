#pragma once

#include <string>

#include <glm/glm.hpp>

#include <heatsink/platform/context.hpp>

namespace heatsink {
	/**
	 * A wrapper for a native window handle. The actual backend used may depend
	 * on the platform being built for (although it is probably GLFW).
	 */
	class window {
	public:
		/**
		 * A window can always be represented by a two-dimensional size.
		 */
		using extents = glm::uvec2;

	private:
		// Allow callbacks to access internal member variables.
		struct callbacks;

	public:
		/**
		 * Create an invalid instance of a window. `is_valid()` is guaranteed to
		 * be `false` for an instance returned from this function. This is
		 * useful when an object must be declared before it can be initialized.
		 */
		static window null();

		/**
		 * Create an offscreen window context. No window will be displayed to
		 * the user, but the window must still interact with the message loop as
		 * normal. Note that no size may be passed, as the window buffer may be
		 * unusable depending on the window manager; use framebuffers instead.
		 */
		static window offscreen(const context&);

	public:
		/**
		 * Create a new window instance. If a window is resizable, care must be
		 * taken to update framebuffers, perspective, etc. when the user alters
		 * the window. On HiDPI screens, the size specifies the apparent size of
		 * the window, that is, the actual pixel size of the framebuffer will be
		 * larger, such that `framebuffer_size = screen_size * dpi_factor`.
		 */
		window(const context&, const std::string& name, extents, bool resize = false);

		// Windows are not trivially copyable, and can only be moved.
		window(const window&) = delete;
		window(window&&) noexcept;
		~window();

		window& operator =(const window&) = delete;
		window& operator =(window&&);

	private:
		// Create an invalid instance of a window (a proxy for `null()`).
		window(std::nullptr_t);

	public:
		/**
		 * Set the OpenGL state machine to use this window for drawing. This is
		 * always called during construction, so it only needs to be called when
		 * handling multiple windows.
		 */
		void use() const;

		/**
		 * Swap window buffers and handle any pending messages for this window.
		 * The return value signals if the window wants to stay alive. `false`
		 * is returned, for example, when a close/quit signal is received.
		 */
		bool flush_buffers() const;
		/**
		 * Output any errors that have accumlated since the last time this
		 * method was invoked. This uses the old-fashioned `glGetError()` to
		 * pop all errors from the OpenGL stack; note that the callback variant
		 * (refer to `message_callback`) supercedes this.
		 */
		void flush_errors() const;

		/**
		 * Check if the window instance is valid. A window should be valid
		 * unless it has thrown an exception or been moved from. Calling any
		 * member functions on an invalid window is undefined behavior, although
		 * it is likely to raise an assertion.
		 */
		bool is_valid() const;
		/**
		 * Retrieve a pointer to the raw window handle. Use with caution; the
		 * pointer is still managed by the instance, and changes made directly
		 * to the pointer may not be reflected.
		 */
		void* get() const;

		/**
		 * Retrieve the apparent size of the window. This is the size passed
		 * during construction, unless it is resizable and has been changed by
		 * the user.
		 */
		extents get_extents() const;
		/**
		 * Retrieve the actual (pixel) size of the window. Refer to `window()`
		 * for how this differs from `get_size()` on HiDPI screens.
		 */
		extents get_framebuffer_extents() const;

	private:
		// The platform backend may not always respect const-correctness; allow
		// the handle to be passed as a mutable pointer regardless of constness.
		mutable void* m_handle;

		// The apparent and actual window sizes.
		extents m_extents;
		extents m_framebuffer_extents;
	};
}
