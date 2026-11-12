#pragma once

#include <vector>
#include <string>
#include <map>
#include "ListenAddress.hpp"
#include "LocationConfig.hpp"

struct ServerConfig
{
	std::vector<ListenAddress>	Listens;
	std::vector<std::string>	ServerNames;
	std::map<int, std::string>	ErrorPages;
	size_t						ClientMaxBodySize;
	std::string					Root;
	std::string					Index;
	std::vector<LocationConfig>	Locations;
};