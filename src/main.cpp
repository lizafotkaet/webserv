#include "http/HttpParser.hpp"
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cout << "Need at least one argument. First argument is the name of whatever \
		you wanna test, the rest is... whatever arguments your test takes" << std::endl;
		return;
	}
	std::string testName = argv[1];
	std::vector<std::string> args(argv + std::min(argc, 2), argv + argc);

	if (testName == "http_parse") HttpParseTesting(args);
	else if (testName == "config_parse") ConfigParseTesting(args);
	// Add more "else if" statements here with different names and function calls to test your own stuff :)
	// "args" will hold any arguments beyond the name, so pass those if you need 'em
	// eg: ./webserv config_parse path/to/config/config.txt
	else std::cout << "No test \""  << testName << "\" found" << std::endl;
}

void HttpParseTesting(std::vector<std::string> args) {
	HttpParser parser = HttpParser();
	parser.AppendBytes("GET testing HTTP/1.0\r\n");
}

void ConfigParseTesting(std::vector<std::string> args) {
	// Empty for now
}