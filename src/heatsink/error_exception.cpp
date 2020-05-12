#include <heatsink/error/exception.hpp>

using namespace std::string_literals;

namespace heatsink {
	exception::exception(const std::string& where, const std::string& what)
	: std::runtime_error("[heatsink::"s + where + "] "s + what) {}
}
