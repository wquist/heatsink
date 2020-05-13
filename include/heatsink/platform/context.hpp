#pragma once

#include <compare>
#include <cstddef>

namespace heatsink {
	/**
	 * An initialized OpenGL context. Set up the OpenGL version and profile,
	 * and load function pointers and extensions through GLEW.
	 */
	class context {
	public:
		/**
		 * An OpenGL version specifier, consisting of a major and minor number,
		 * with comparison operators for convenience.
		 */
		struct version {
		public:
			auto operator <=>(const version&) const = default;

		public:
			// Note that the order of the members is important for proper
			// functioning of the spaceship operator.
			std::size_t major;
			std::size_t minor;
		};

	public:
		/**
		 * An OpenGL context type. The profile determines how compatibility and
		 * deprecation should be handled.
		 * - A core profile contains features for 3.3 and above, with some older
		 *   functionality still available for use.
		 * - A strict profile (also called a forward-compatible profile)
		 *   completely removes older functionality, leaving only the post-3.3
		 *   features.
		 * - A compatibility profile brings back functionality removed after
		 *   version 3.1, effectively reversing these deprecations.
		 * - "any" allows the host system to choose the appropriate profile.
		 * Note that on Mac OS, only the strict profile is available (although
		 * `any` should work, choosing `strict` itself).
		 */
		enum class profile {
			core,
			strict,
			compatibility,
			any
		};

	public:
		/**
		 * Specify the information for a potential OpenGL context. Enabling
		 * `debug` will set the context flag and install message handlers if
		 * appropriate for the version.
		 */
		context(version = {3,3}, profile = profile::any, bool debug = true);
		/**
		 * Retrieve version/profile information from the given window. Using
		 * this constructor also allows contexts to be shared between windows.
		 */
		context(const class window&);

		// A context is non-copyable; it may manage platform-specific resources.
		context(const context&) = delete;
		context(context&&) noexcept;
		~context();

		context& operator =(const context&) = delete;
		context& operator =(context&&);

	public:
		/**
		 * Retrieve the platform-specific context handle, if one exists. This
		 * is probably `nullptr` unless the context was created from an existing
		 * window/context instance.
		 */
		void* get() const;

		/**
		 * Retrieve the version of the created OpenGL context. This may not be
		 * the version specified in the constructor, as that is only a minimum;
		 * OpenGL is permitted to choose any higher version number.
		 */
		version get_version() const;
		/**
		 * Retrieve the profile used in the created OpenGL context. Unless `any`
		 * was used, this is always what was passed to the constructor; it is a
		 * fatal error if the context cannot be created with the given profile.
		 */
		profile get_profile() const;

		/**
		 * Check if this should be a debug context. This allows for extra error
		 * checks and messages within the OpenGL API.
		 */
		bool is_debug() const;

	private:
		// The platform backend may not always respect const-correctness; allow
		// the handle to be passed as a mutable pointer regardless of constness.
		mutable void* m_handle;

		// Version/profile information is the same across platforms.
		version m_version;
		profile m_profile;
		bool m_debug;
	};
}
