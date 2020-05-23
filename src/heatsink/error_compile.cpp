#include <heatsink/error/compile.hpp>

#include <cassert>
#include <iomanip>
#include <regex>
#include <sstream>

namespace {
	template<class GetLength, class GetLog>
	std::string read_log(GLuint name, GetLength get_length, GetLog get_log) {
		GLint length;
		get_length(name, GL_INFO_LOG_LENGTH, &length);

		assert(length);
		std::string str(length, '\0');

		get_log(name, length, nullptr, str.data());
		return str;
	}

	void write_log(std::ostream& os, const std::string& msg, const std::string& from) {
		// Apply a regex to each line of log output, since Nvidia and ATI use
		// different formatting. For example,
		//   Nvidia:
		//     ```
		//     0(35) : error C3008: unknown layout specifier 'row_major'
		//     0(36) : error C3008: unknown layout specifier 'row_major'
		//     ```
		//   ATI:
		//     ```
		//     ERROR: 0:83: error(#143) Undeclared identifier blinn_phong
		//     ERROR: 0:83: error(#132) Syntax error: 'term' parse error
		//     ```
		// Although there is no standard format, they both contain the same
		// information: the ID/line number, and an error string. The regex
		// can be broken down as follows:
		//   `\D*`        : ignore everything up to the first number.
		//   `\d+`        : extract the file ID.
		//   `.`          : ignore the separator (`:` for Nvidia, `(` for ATI).
		//   `(\d+)`      : capture the error line number.
		//   `\D*`        : ignore up to the error code number.
		//   `[^a-zA-Z.]*`: skip everything up to the actual error message.
		//   `(.*)`       : capture the error message (the rest of the line).
		// Note that the error message can include a '.' at the start, since
		// Nvidia starts their multiline messages with "...".
		std::regex regex("^\\D*\\d+.(\\d+)\\D*[^a-zA-Z.]*(.*)$");
		unsigned long last_position{};

		std::istringstream iss(msg);
		for (std::string line; std::getline(iss, line);) {
			std::smatch matches;
			// Skip any lines that do not match; ATI uses some extra output
			// describing the type of shader and summing the number of errors.
			if (!std::regex_match(line, matches, regex))
				continue;

			// Indent the line slightly.
			os << "  ";

			auto position = std::stoul(matches[1]);
			// When multiple errors are on the same line (or it is a multiline
			// message) there is no need to output the `from` or position.
			if (position == last_position)
				os << std::setw(from.length() + 7) << " ";
			else
				os << from << "(" << std::setw(3) << std::string(matches[1]) << "): ";

			last_position = position;
			os << std::string(matches[2]) << std::endl;
		}
	}
}

namespace heatsink::gl {
	void write_shader_log(std::ostream& os, GLuint name, const std::string& from) {
		assert(name);

		auto msg = read_log(name, glGetShaderiv, glGetShaderInfoLog);
		write_log(os, msg, from);
	}

	void write_program_log(std::ostream& os, GLuint name, const std::string& from) {
		assert(name);

		auto msg = read_log(name, glGetProgramiv, glGetProgramInfoLog);
		write_log(os, msg, from);
	}
}
