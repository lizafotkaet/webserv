#include "include/http/HttpParser.hpp"

int main(int argc, char** argv) {
	HttpParser parser = HttpParser();
	parser.AppendBytes("GET testing HTTP/1.0\r\n");
	//path: testing
}