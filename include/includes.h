#pragma once

#include <netdb.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <iostream>
#include <string_view>
#include <unordered_map>
#include <map>
#include <vector>
#include <array>


#include "http/HttpParser.hpp"
#include "http/HttpRequest.hpp"
#include "../enum/http/HttpMethod.hpp"
#include "../enum/http/HttpParserResultState.hpp"
#include "../enum/http/HttpParserState.hpp"
#include "config/ListenAddress.hpp"
#include "config/LocationConfig.hpp"
#include "config/ServerConfig.hpp"
#include "config/WebservConfig.hpp"
#include "server/Client.h"
#include "server/Server.h"
#include "server/constants.h"

