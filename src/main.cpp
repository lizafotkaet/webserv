// #include "include/http/HttpParser.hpp"

// int main(int argc, char** argv) {
// 	HttpParser parser = HttpParser();
// 	parser.AppendBytes("GET testing HTTP/1.0\r\n");
// 	//path: testing
// }

#include "includes.h"

int main(int ac, char** av) {

	void(ac);
	void(av);
	if (ac != 2){
		std::cout << "Specify config file\n";
		return 1;
	}


	HttpRequest request;
}
