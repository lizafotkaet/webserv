#pragma once
#include <charconv>
#include <optional>
#include <string_view>
#include <system_error>

class WSConvert {
	public:
	// Tries to convert a string to a given type of number, using any base
	// Note: AI-Assisted function, thanks Claude
	template <typename T>
	static std::optional<T> TryParseNum(std::string_view str, int base = 10, std::size_t length = std::string_view::npos);
};

template <typename T>
std::optional<T> WSConvert::TryParseNum(std::string_view str, int base, std::size_t length)
{
	str = str.substr(0, length);
	T value{};
	const char* begin = str.data();
	const char* end = begin + str.size();
	auto [ptr, ec] = std::from_chars(begin, end, value, base);
	if (ec != std::errc() || ptr != end)
		return std::nullopt;
	return value;
};