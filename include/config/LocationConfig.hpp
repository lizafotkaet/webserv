#pragma once

#include <string>
#include <set>
#include <optional>
#include <filesystem>
#include <unordered_map>

struct LocationConfig {
	std::string										Path;
	std::set<std::string>							AllowedMethods;
	std::optional<std::string>						Root;
	std::optional<std::string>						Index;
	std::optional<bool>								Autoindex;
	std::optional<size_t>							ClientMaxBodySize;
	std::optional<std::pair<int, std::string>>		Redirect;
	std::optional<std::filesystem::path>			UploadStore;
	std::unordered_map<std::string, std::string>	CgiHandlers;
};