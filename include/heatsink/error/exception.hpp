#pragma once

#include <stdexcept>
#include <string>

namespace heatsink {
	/**
	 * A heatsink-related exception. Derives from `runtime_error`, but takes in
	 * location information in addition to the actual error, formatting messages
	 * like "[heatsink:window] could not create window."
	 */
	class exception : public std::runtime_error {
	public:
		/**
		 * Create a new exception from "where" the error occurs (probably the
		 * name of the source class) and "what" went wrong.
		 */
		explicit exception(const std::string& where, const std::string& what);
	};
}
