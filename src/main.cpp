// #include "include/http/HttpParser.hpp"

// int main(int argc, char** argv) {
// 	HttpParser parser = HttpParser();
// 	parser.AppendBytes("GET testing HTTP/1.0\r\n");
// 	//path: testing
// }

#include "includes.h"

// Temporary until the config parser exists: swap this for parseConfig(argv[1])
static std::vector<ServerConfig> makeTestConfig()
{
	ServerConfig a;
	a.Listens = { {"0.0.0.0", 8080}, {"127.0.0.1", 8081} };
	a.ServerNames = { "localhost" };
	a.ClientMaxBodySize = 1024 * 1024;
	a.Root = "./www";
	a.Index = "index.html";
	a.ErrorPages[404] = "/errors/404.html";

	ServerConfig b;
	b.Listens = { {"0.0.0.0", 9090} };
	b.ClientMaxBodySize = 1024 * 1024;
	b.Root = "./www2";
	b.Index = "index.html";

	return { a, b };
}

/*
	this shit for signals to deal with later!!!
*/
static Server* g_server = nullptr;

static void onSignal(int)
{
	if (g_server)
		g_server->stop();
}


int main()
{
	std::signal(SIGINT, onSignal);
	std::signal(SIGPIPE, SIG_IGN);   // a client closing mid-send must not kill us

	try {
		std::vector<ServerConfig> configs = makeTestConfig();
		Server server(configs);
		g_server = &server;
		server.run();
		g_server = nullptr;
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}

