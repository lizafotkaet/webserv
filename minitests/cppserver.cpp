#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>

int main()
{
    // creating socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // specifying the address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9898);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // binding socket.
    bind(serverSocket, (struct sockaddr*)&serverAddress,
         sizeof(serverAddress));

    // listening to the assigned socket
    listen(serverSocket, 5);

	int epoll_fd = epoll_create(10);

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = serverSocket;

	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocket, &event);
	int max_event_count = 100;
	int timeout = -1;
	struct epoll_event events[max_event_count];

	while (1) {
		int eventCount = epoll_wait(epoll_fd, events, max_event_count, timeout);
		for (int i = 0; i < eventCount; i++) {
			if (events[i].data.fd == serverSocket) {
				// new connection was found, we need to accept in this if statement
			}
			else if (events[i].events & EPOLLIN) {
				// existing connection is ready to give us data (their HTTP request)
				// so here we read that data using read() and store it somewhere for parsing later
			}
			else if (events[i].events & EPOLLOUT) {
				// existing connection is done sending us data and is now ready to receive a response
				// so here we send our response after we built one using send() (it's a HTTP response)
			}
		}
	}
    // closing the socket.
    close(serverSocket);

    return 0;
}