#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <vector>
#include <array>
#include <netdb.h>


using namespace std;

int main()
{
    // specifying the address
	addrinfo	hints{};
	addrinfo*	res{nullptr};
    // sockaddr_in serverAddress;
	hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

	std::string host{"127.0.0.1"}, port{"6969"};
	getaddrinfo(host.c_str(), port.c_str(), &hints, &res);

    // creating socket
	int serverSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	fcntl(serverSocket, F_SETFL, O_NONBLOCK);

    // binding socket.
    bind(serverSocket, res->ai_addr, res->ai_addrlen);

	cout << "Socket bound yaaaaay...\n\n";

    // listening to the assigned socket
    listen(serverSocket, 5);

	cout << "Listening...\n";

	int epoll_fd = epoll_create(10);

	struct epoll_event event;
	event.events = EPOLLIN; // incoming connection??? i.e. READABLE
	event.data.fd = serverSocket;

	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocket, &event);

	const int maxevents = 1024;
	const int timeout = -1;
	struct epoll_event events[maxevents];
    // accepting connection request
	while (1){
		int num = epoll_wait(epoll_fd, events, maxevents, timeout);
		for (int i{}; i < num; i++){
			int fd = events[i].data.fd;

			if (fd == serverSocket){
				int clientSocket = accept(serverSocket, nullptr, nullptr);
				fcntl(clientSocket, F_SETFL, O_NONBLOCK);
				struct epoll_event event;
				event.events = EPOLLIN;
				event.data.fd = clientSocket;
				epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientSocket, &event);
			}
			else {
				char buffer[1024] = {0};
				const char* message = "Hello, client!";
				int n = recv(fd, buffer, sizeof(buffer), 0);
				if (n <= 0){
					close(fd);
					continue ;
				}
				cout << "Message from client: " << buffer << endl;
				send(fd, message, strlen(message), 0);
			}
		}
		// recieving data
		// close(clientSocket);
	}

	freeaddrinfo(res); // do we need this tho????
    // closing the socket.
    close(serverSocket);

    return 0;
}
