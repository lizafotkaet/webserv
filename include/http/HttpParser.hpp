#pragma once
#include <string>
#include "http/HttpParserState.hpp"
#include "http/HttpParserResultState.hpp"
#include "http/HttpParserBodyType.hpp"
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
	private:
	static bool isValidHttpPathChar(char c);
	static bool isTchar(unsigned char c);
	static bool isFieldValueChar(unsigned char c);
	static bool isValidHeaderName(std::string s);
	static bool isValidHeaderValue(std::string s);
	static bool isUnrepeatableHeader(std::string headerName);
	static bool isRecognizedHeader(std::string headerName);
	inline static const std::string HTTP_PATH_SPECIAL_CHARS = "-_.~/!$&'()*+,;=:@%?";
	inline static const std::size_t HTTP_HEADER_NAME_MAXLEN = 255;
	inline static const std::size_t HTTP_HEADER_VALUE_MAXLEN = 1024;
	HttpParserBodyType bodyType;
	bool contentLengthSet;
	std::size_t contentLength;
	void tryContinueParsing();
	void parseMethod();
	void parsePath();
	void parseHttpVersion();
	void parseHeader();
	void parseChunkedBody();
	void parseNormalBody();
	void parseBody();
	void setBodyType();
};