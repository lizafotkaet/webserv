#pragma once
enum class HttpParserState {
	METHOD,
	PATH,
	HTTPVERSION,
	HEADERS,
	BODY
};