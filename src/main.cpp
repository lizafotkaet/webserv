#include "http/HttpParser.hpp"
#include <iostream>
#include <string>
#include <vector>

#include <algorithm>
#include <fstream>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

// AI-assisted function for early testing, takes path to file and a parser
// reads it and splits it up into a random amount of pieces to simulate how
// the data would come in parts from our event loop thing
// each piece gets fed into the parser, which continuously tries to parse
// until it's either invalid or finished
bool FeedFileInRandomParts(HttpParser& parser, const std::string& path)
{
	std::ifstream file(path, std::ios::binary);
	if (!file) {
		//std::cout << "No such request file\r\n";
		return false;
	}
	std::ostringstream stream;
	stream << file.rdbuf();
	std::string data = stream.str();
	if (data.empty()) {
		//std::cout << "Request file is empty\r\n";
		return false;
	}

	std::mt19937 rng{std::random_device{}()};
	std::size_t partCount = std::uniform_int_distribution<std::size_t>(1, data.size())(rng);

	std::vector<std::size_t> cuts(data.size() - 1);
	std::iota(cuts.begin(), cuts.end(), 1);
	std::shuffle(cuts.begin(), cuts.end(), rng);
	cuts.resize(partCount - 1);
	std::sort(cuts.begin(), cuts.end());
	cuts.push_back(data.size());

	std::size_t start = 0;
	for (std::size_t cut : cuts) {
		if (parser.resultState == HttpParserResultState::INVALID) {
			//std::cout << "Invalid request\r\n";
			return false;
		}
		std::string part = data.substr(start, cut - start);
		parser.AppendBytes(part.data());
		start = cut;
	}
	return true;
}

// AI-assisted as well, temporary, just no time to write my own tests really
void HttpParseTesting(std::vector<std::string> args) {
    if (args.size() != 1) {
        std::cout << "Wrong amount of args\r\n";
        return;
    }

    std::filesystem::path path(args[0]);
    std::error_code ec;

    auto runOne = [](const std::string& file, const std::string& filename) {
        HttpParser parser;
		bool feedRes = FeedFileInRandomParts(parser, file);
		bool success = (int)parser.resultState == 1;
		if (feedRes && success) {
			if (filename.starts_with("ok")) std::cout << "✅ " << filename << "\r\n";
			else std::cout << "⛔ " << filename << "\r\n";
		}
		else {
			if (filename.starts_with("ok")) std::cout << "⛔ " << filename << "\r\n";
			else std::cout << "✅ " << filename << "\r\n";
		}
    };

    if (std::filesystem::is_directory(path, ec)) {
        std::vector<std::filesystem::path> files;
        for (const auto& entry : std::filesystem::directory_iterator(path, ec)) {
            if (entry.is_regular_file())
                files.push_back(entry.path());
        }
        std::sort(files.begin(), files.end());

        for (const auto& file : files) {
            runOne(file.string(), file.filename().string());
        }
    } else {
        runOne(args[0], std::filesystem::path(args[0]).filename().string());
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
