#include "../../include/includes.h"

/*

work in progress!!

*/

int createListenSocket(const std::string &host, const std::string &port)
{
    addrinfo hints{};                    // value-init zeroes everything, no memset needed
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    addrinfo *raw = nullptr;
    if (int err = getaddrinfo(host.c_str(), port.c_str(), &hints, &raw); err != 0)
        throw std::runtime_error(std::string("getaddrinfo: ") + gai_strerror(err));

    // freeaddrinfo runs automatically when res goes out of scope, even on throw
    std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> res(raw, &freeaddrinfo);

    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0)
        throw std::runtime_error("socket failed");

    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    if (bind(fd, res->ai_addr, res->ai_addrlen) < 0 || listen(fd, SOMAXCONN) < 0) {
        close(fd);
        throw std::runtime_error("bind/listen failed on " + host + ":" + port);
    }

    fcntl(fd, F_SETFL, O_NONBLOCK);
    return fd;
}
