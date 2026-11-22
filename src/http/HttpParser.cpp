#include <string>
#include "http/HttpParserState.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpParser.hpp"
#include "helpers/WSConvert.hpp"
#include "helpers/WSNumbers.hpp"
#include <iostream>
#include <cstring>
#include <optional>

HttpParser::HttpParser() : state(HttpParserState::METHOD), bodyType(HttpParserBodyType::UNKNOWN) {}

// Takes parts of an HTTP request and builds them up into one string
void HttpParser::AppendBytes(char bytes[]) { //TODO: Make it just use std::string lol
	std::string str(bytes);
	this->buffer += str;
	// Since new data has come in, we can try to continue parsing again
	// TODO: MAYBE, but not likely, this is the place to check whether we even want to continue trying to parse
	this->tryContinueParsing();
}

void HttpParser::tryContinueParsing() { //TODO: implement the rest of parsing
	switch (this->state) {
		case HttpParserState::METHOD: this->parseMethod(); break;
		case HttpParserState::PATH: this->parsePath(); break;
		case HttpParserState::HTTPVERSION: this->parseHttpVersion(); break;
		case HttpParserState::HEADERS: this->parseHeader(); break;
		case HttpParserState::BODY: this->parseBody(); break;
		default: return;
	}
}

// Parses the HTTP method out of a (partial) HTTP request and inserts it into the eventual resulting HttpRequest class
void HttpParser::parseMethod() {
	// Use maximum size in combination with looking for a space to determine whether 
	// the received data is far enough advanced to contain any valid HTTP method
	if (this->buffer.size() < 7 && this->buffer.find_first_of(" ") == std::string::npos) return;
	// Set resulting methods based on what we find in the buffer string
	if (this->buffer.starts_with("GET ")) this->result.Method = HttpMethod::GET;
	else if (this->buffer.starts_with("POST ")) this->result.Method = HttpMethod::POST;
	else if (this->buffer.starts_with("DELETE ")) this->result.Method = HttpMethod::DELETE;
	// Handle invalid HTTP method
	else {
		this->result.Method = HttpMethod::INVALID;
		this->resultState = HttpParserResultState::INVALID;
		return;
	}
	// This is only reached if the HTTP method is valid
	// Remove the now already parsed data from the buffer string
	this->buffer.erase(0, this->buffer.find_first_of(" ") + 1);
	// Update our parser state
	this->state = HttpParserState::PATH;
	// Call the next part of the parser
	this->parsePath();
}
// Parses the HTTP path out of a (partial) HTTP request and inserts it into the eventual resulting HttpRequest class
// NOTE: This does not do full URL validation, it is only a basic check to see if the path consists of allowed characters
void HttpParser::parsePath() {
	// Holds the size of the resulting path string, if a valid one is found
	int pathSize = 0;
	// State of the path parsing, defaulting to unfinished
	HttpParserResultState pathState = HttpParserResultState::UNFINISHED;
	// Loop through all the characters in the buffer string
	// TODO: We need a configurable maximum path length so we don't potentially read forever
	for (unsigned int i = 0; i < this->buffer.size(); i++) {
		// If we run into a space, we potentially have the end of the path
		if (this->buffer[i] == ' ') {
			// .. but if the first character is a space, that's not a valid path, so we check for that first
			if (i > 0) pathState = HttpParserResultState::VALID;
			// if it was in fact invalid, we handle that here
			else {
				pathState = HttpParserResultState::INVALID;
				this->resultState = HttpParserResultState::INVALID;
			}
			// Because validity of the path is determined at this point, we can exit the loop here
			break;
		}
		// Checks if the current character is a disallowed one, in which case we do invalid request handling again
		if (!isValidHttpPathChar(this->buffer[i])) {
			pathState = HttpParserResultState::INVALID;
			this->resultState = HttpParserResultState::INVALID;
			break;
		}
		// Increase path size after the potential breaks, which excludes the trailing space in case of a valid path
		pathSize++;
	}
	// After the loop we check whether the path is valid
	if (pathState == HttpParserResultState::VALID) {
		// if so, we set the resulting HttpRequest class path to the path we found
		this->result.Path = this->buffer.substr(0, pathSize);
		this->buffer.erase(0, pathSize + 1);
		// temporarily print out the path for testing purposes
		// TODO: remove once not required anymore
		this->state = HttpParserState::HTTPVERSION;
		this->parseHttpVersion();
	}
}

void HttpParser::parseHttpVersion() {
	if (this->buffer.starts_with("HTTP/1.0\r\n") || this->buffer.starts_with("HTTP/1.1\r\n")) {
		this->result.HttpVersion = "1.0"; // TODO: Possibly just.... either make this an int or get rid of it completely since it's the only thing we support
		this->state = HttpParserState::HEADERS;
		this->buffer.erase(0, 10); // remove the http method including the CRLF
		this->parseHeader();
	}
	else this->resultState = HttpParserResultState::INVALID;
}

void HttpParser::parseHeader() { //TODO: Check if maybe we just make the parse result state the return type of these parse functions instead
	// Check if we know there's at least a full line in the buffer
	// TODO: Also investigate std::string_view as it may be a more efficient way to do what we do in most parsing
	std::size_t headerEndLoc = this->buffer.find_first_of("\r\n");
	if (headerEndLoc != std::string::npos) {
		if (headerEndLoc == 0) { //TODO: replace with find_first_of \r\n\r\n and compare to headerend
			this->state = HttpParserState::BODY;
			// Remove from buffer the CRLF that signifies the end of headers
			this->buffer.erase(0, 2);
			this->parseBody();
			return;
		}
		// +2 for the colon and space separating them, leading and trailing whitespace must be trimmed but we will count it towards max len
		// this is only a quick sanity check and does not guarantee header and name both adhere to their max lengths
		if (headerEndLoc > (HttpParser::HTTP_HEADER_NAME_MAXLEN + HttpParser::HTTP_HEADER_VALUE_MAXLEN + 2)) {
			this->resultState = HttpParserResultState::INVALID; // TODO: Consider adding a "why is this invalid" because there's a lot of ways
			return;
		}
		// Find the colon in header to know where to split
		std::size_t colonPos = this->buffer.find_first_of(":");
		if (colonPos == std::string::npos) {
			this->resultState = HttpParserResultState::INVALID;
			return;
		}
		std::string headerName = this->buffer.substr(0, colonPos);
		// Inline tolower, suurely we won't need this elsewhere!
		for (char& c : headerName)
		{
			if (c >= 'A' && c <= 'Z')
				c += 'a' - 'A';
		}
		std::string headerValue = "";
		// Check that there actually is a header value and the header line doesn't end directly after the colon
		// Empty values are allowed but we don't wanna try to read if there is nothin'
		if (colonPos + 1 < headerEndLoc) {
			headerValue = this->buffer.substr(colonPos + 1, headerEndLoc - colonPos - 1);
			std::size_t skipLeadingSpace = headerValue.find_first_not_of(" \t");
			std::size_t skipTrailingSpace = headerValue.find_last_not_of(" \t");
			// If value consists of only whitespace, keep it empty
			if (skipLeadingSpace == std::string::npos || skipTrailingSpace == std::string::npos) {
				headerValue = "";
			}
			else
				headerValue = headerValue.substr(skipLeadingSpace, skipTrailingSpace - skipLeadingSpace + 1);
		}
		// Validate both the header name and the header value
		if (!isValidHeaderName(headerName) || !isValidHeaderValue(headerValue)) {
			this->resultState = HttpParserResultState::INVALID;
			return;
		}
		// Try to insert the header name+value into the request's map
		auto [iter, inserted] = this->result.Headers.try_emplace(headerName, headerValue);
		// Check if it inserted. If not, that header name already exists
		if (!inserted) {
			// If it's one of the few headers that can not be repeated, mark request as invalid
			if (isUnrepeatableHeader(headerName)) {
				this->resultState = HttpParserResultState::INVALID;
				return;
			}
			// If repeatable, just append the new value (comma-separated)
			iter->second.append(", ").append(headerValue);
		}
		// remove the http method including the CRLF
		this->buffer.erase(0, headerEndLoc + 2);
		// call self again, should be fine.. it either removes stuff, marks as invalid, or finds CRLF at index 0 and then leaves
		this->parseHeader();
	}
	else {
		// TODO: I don't really remember off the top of my head if this *can* happen or if it's like an edge case
		// Just gotta look into what to do if so, probably invalid but might also just be no body or something? 90% invalid tho
		// At the very least we get rid of the print lol it's here temporarily!
		std::cout << "just no CRLF found\n";
	}
}

void HttpParser::setBodyType() {
	auto teIter = this->result.Headers.find("transfer-encoding");
	auto clIter = this->result.Headers.find("content-length");
	// Invalid if both Content-Length and Transfer-Encoding are present
	if (teIter != this->result.Headers.end() && clIter != this->result.Headers.end()) {
			this->resultState = HttpParserResultState::INVALID;
			return;
	}
	// Handle Transfer-Encoding
	if (teIter != this->result.Headers.end()) {
		// Check if value is exact match, anything else is invalid
		if (teIter->second == "chunked") {
			this->bodyType = HttpParserBodyType::CHUNKED;
		}
		else {
			this->resultState = HttpParserResultState::INVALID;
			return;
		}
	}
	else if (clIter != this->result.Headers.end()) {
		std::optional<std::size_t> tryLen = WSConvert::TryParseNum<std::size_t>(clIter->second);
		if (!tryLen) {
			this->resultState = HttpParserResultState::INVALID;
			return;
		}
		// TODO: Implement a "max content length" thing at some point and enforce it here?
		this->contentLength = tryLen.value();
		this->bodyType = HttpParserBodyType::NORMAL;
	}
	else this->bodyType = HttpParserBodyType::EMPTY;
	// Try parse again as our bodyType should no longer be unknown
	this->parseBody();
}

void HttpParser::parseChunkedBody() {
	size_t index = 0;
	while (WSNumbers::IsDigit(this->buffer[index]))
		index++;
	// Valid chunk size found but the buffer is too small to hold the CRLF to end chunk size line
	// TODO: Probably have a "max chunk size" thing at some point and enforce it here?
	if (index == 0) {
		// Could just be an incomplete chunk line, so exit out for now
		if (buffer.length() == 0) return;
		// If length wasn't 0 it means we just found some non-number character and it's invalid
		this->resultState = HttpParserResultState::INVALID;
		return;
	}
	if (this->buffer.length() < index + 2) return;
	// Chunk size followed by non CRLF = invalid
	// TODO: Probably needs some "IsCRLF" helper function cus this is tedious
	if (this->buffer[index] != '\r' || this->buffer[index + 1] != '\n') {
		this->resultState = HttpParserResultState::INVALID;
		return;
	}
	std::optional<std::size_t> tryChunkSize = WSConvert::TryParseNum<std::size_t>(this->buffer, 10, index);
	// If number parsing failed, request is also invalid. Think this can only happen on overflow
	// TODO: Probably check against a "max body size" if we add this chunk to the existing body, idk
	// NOTE: The above todo is awkwardly placed and should go further down just cba rn
	if (!tryChunkSize) {
		this->resultState = HttpParserResultState::INVALID;
		return;
	}
	if (tryChunkSize.value() == 0) {
		if (this->buffer.length() == index + 4 && this->buffer.ends_with("\r\n\r\n"))
			this->resultState = HttpParserResultState::VALID;
		else this->resultState = HttpParserResultState::INVALID;
		return;
	}
	// If smaller than the strlen of chunkSize + the actual size of the chunk + 4 for 2 CRLF's, invalid
	if (this->buffer.length() < index + 4 + tryChunkSize.value()) {
		this->resultState = HttpParserResultState::INVALID;
		return;
	}
	size_t chunkEnd = index + 2 + tryChunkSize.value();
	// Invalid if chunk does not end with CRLF
	if (this->buffer[chunkEnd] != '\r' || this->buffer[chunkEnd + 1] != '\n') {
		this->resultState = HttpParserResultState::INVALID;
		return;
	}
	// Append the chunk to our result body
	this->result.Body += this->buffer.substr(index + 2, tryChunkSize.value());
	// Remove from buffer the chunk length line and chunk itself
	this->buffer.erase(0, index + 4 + tryChunkSize.value());
	// No 0-size chunk yet, continue trying to parse
	parseChunkedBody();
}

void HttpParser::parseNormalBody() {
	// Leave buffer untouched until the whole body is in there
	if (this->buffer.length() < this->contentLength) return;
	this->result.Body = this->buffer.substr(0, this->contentLength);
	this->resultState = HttpParserResultState::VALID;
	return;
}

void HttpParser::parseBody() {
	if (this->bodyType == HttpParserBodyType::UNKNOWN)
		setBodyType();
	else if (this->bodyType == HttpParserBodyType::CHUNKED)
		parseChunkedBody();
	else if (this->bodyType == HttpParserBodyType::NORMAL)
		parseNormalBody();
	else if (this->bodyType == HttpParserBodyType::EMPTY) {
		this->resultState = HttpParserResultState::VALID;
	}
}

bool HttpParser::isUnrepeatableHeader(std::string headerName) {
	return (headerName == "host"
		||headerName == "content-length"
		|| headerName == "content-type");
}

bool HttpParser::isRecognizedHeader(std::string headerName) {
	return (headerName == "host"
		||headerName == "content-length"
		|| headerName == "content-type"
		|| headerName == "transfer-encoding");
}

// Checks whether a given character is allowed in a HTTP path
bool HttpParser::isValidHttpPathChar(char c) {
	return std::isalnum(c)
		|| HTTP_PATH_SPECIAL_CHARS.contains(c);
}

// Checks if a character is one that is allowed in a HTTP header name
bool HttpParser::isTchar(unsigned char c)
{
	if (std::isalnum(c))
		return true;
	return std::strchr("!#$%&'*+-.^_`|~", c) != nullptr && c != '\0';
}

// Checks if a character is one that is allowed in a HTTP header value
bool HttpParser::isFieldValueChar(unsigned char c)
{
	return c == '\t' || (c >= 0x20 && c != 0x7F);
}

// Checks if a string is a valid HTTP header name
bool HttpParser::isValidHeaderName(std::string s) {
	for (unsigned char c : s) {
		if (!isTchar(c)) return false;
	}
	return true;
}

// Checks if a string is a valid HTTP header value
bool HttpParser::isValidHeaderValue(std::string s) {
	for (unsigned char c : s) {
		if (!isFieldValueChar(c)) return false;
	}
	return true;
}

