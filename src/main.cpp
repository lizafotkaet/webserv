#include "http/HttpParser.hpp"
#include <iostream>
#include <string>
#include <vector>

void HttpParseTesting(std::vector<std::string> args) {
	HttpParser parser = HttpParser();
	//parser.AppendBytes("GET /testing HTTP/1.0\r\nHost:   mika.com\r\nContent-Length:3\r\nContent-Fakeheader: lmao\r\nContent-Fakeheader: xdxd\r\n\r\nabc");
	parser.AppendBytes("GET /testing HTTP/1.0\r\nHost:   mika.com\r\nTransfer-Encoding:chunked\r\nContent-Fakeheader: lmao\r\nContent-Fakeheader: xdxd\r\n\r\n3\r\nabc\r\n4\r\nlmao\r\n0\r\n\r\n");
	if (parser.resultState == HttpParserResultState::VALID) {
		std::cout << "--Start\tChunked\tBody--\r\n" << parser.result.Body << "\r\n--End\tChunked\tBody--\r\n";
	}
	else if (parser.resultState == HttpParserResultState::INVALID) {
		std::cout << "Invalid request\r\n";
	}
}

void ConfigParseTesting(std::vector<std::string> args) {
	// Empty for now
}

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cout << "Need at least one argument. First argument is the name of whatever \
		you wanna test, the rest is... whatever arguments your test takes" << std::endl;
		return 1;
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
