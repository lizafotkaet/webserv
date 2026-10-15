#pragma once
#include <vector>
#include <utility>
#include <string>
#include "HttpMethod.hpp"

class HttpRequest {
	public:
	HttpMethod Method;
	std::vector<std::pair<std::string, std::string>> Headers;
}