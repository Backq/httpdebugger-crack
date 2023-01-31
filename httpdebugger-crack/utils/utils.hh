#pragma once
#include <string>
#include <regex>

#pragma warning(disable: 4996)

namespace utils {
	inline std::string parsed_version;
	inline std::string generated_key;
	inline std::string serial_number;

	extern std::string get_regex(std::regex rx, std::string text);
	extern void get_serial_number();
	extern void get_app_version();
	extern void create_key();
	extern void write_key();
};
