#pragma once
#include <string>
#include "http/HttpParserState.hpp"
#include "http/HttpParserResultState.hpp"
#include "HttpRequest.hpp"

class HttpParser {
	public:
	// We append all the received buffers to this string so it becomes whole
	std::string buffer;
	// The state of parsing we're in, used to keep track of what is next
	HttpParserState state;
	// The state of the result from parsing, used to keep track if whether we need to continue parsing
	HttpParserResultState resultState;
	// The resulting HttpRequest class, will be filled with everything we parse
	HttpRequest result;

	// Used by the core loop to add received bytes onto the request string
	HttpParser();
	void AppendBytes(char bytes[]);
	static bool IsValidHttpPathChar(char c);
	private:
	inline static const std::string HTTP_PATH_SPECIAL_CHARS = "-_.~/!$&'()*+,;=:@%?";
	void tryContinueParsing();
	void parseMethod();
	void parsePath();
};