#pragma once

#include <cstdint>
#include <string>

struct ListenAddress
{
	std::string Host;
	std::uint16_t Port;
};