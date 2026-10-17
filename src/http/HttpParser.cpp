#include <string>
#include "http/HttpParserState.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpParser.hpp"
#include <iostream>

HttpParser::HttpParser() : state(HttpParserState::METHOD) {}

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
		// case PARSE_STATE_HTTPVERSION: this->parseHttpVersion(); break;
		// case PARSE_STATE_HEADERS: this->parseHeaders(); break;
		// case PARSE_STATE_BODY: this->parseBody(); break;
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
	for (int i = 0; i < this->buffer.size(); i++) {
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
		if (!IsValidHttpPathChar(this->buffer[i])) {
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
		// temporarily print out the path for testing purposes
		// TODO: remove once not required anymore
		std::cout << "path found: \"" << this->result.Path <<  "\"" << std::endl;
	}
}

// Checks whether a given character is allowed in a HTTP path
bool HttpParser::IsValidHttpPathChar(char c) {
	return std::isalnum(c)
		|| HTTP_PATH_SPECIAL_CHARS.contains(c);
}


