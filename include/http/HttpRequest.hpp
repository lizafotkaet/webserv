#pragma once
#include <map>
#include <string>
#include "http/HttpMethod.hpp"

class HttpRequest {
	public:
	HttpMethod Method;
	std::string Path;
	std::string HttpVersion;
	std::map<std::string, std::string> Headers;
	std::string Body;
};