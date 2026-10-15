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
				// New connection to the server socket was found
				// We need to accept it
				// TODO: Accept can probably return -1 on failure to accept in which case we should handle that
				int clientSocket = accept(events[i].data.fd, nullptr, nullptr);

				// Create new event for when this new client socket is ready to let us read data
				// Event is called EPOLLIN, which we store alongside the file descriptor of the socket
				struct epoll_event event;
				event.events = EPOLLIN;
				event.data.fd = clientSocket;

				// epoll_ctl is the control function which we can use to modify our epoll setup
				// EPOLL_CTL_ADD is one of the actions, and it lets us add a new socket to the setup
				epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientSocket, &event);
				// For testing purposes, we are printing out a message to show the file descriptor of the newly connected client
				std::cout << "Client with FD " << clientSocket << " connected!\n";
			}
			else if (events[i].events & EPOLLIN) {
				// existing connection is ready to give us data (their HTTP request)
				// so here we read that data using read() and store it somewhere for parsing later

				// The event we're currently at in the loop holds a file descriptor, this variable just makes it easier to access
				int clientFd = events[i].data.fd;
				// We create a buffer of characters to start reading the client's request into
				char buffer[1024] = { 0 };
				// Use recv to receive (part of) the request, we read them into buffer and with a maximum size of 1024 (which is the size of buffer)
				ssize_t readBytes = recv(clientFd, buffer, sizeof(buffer), 0);
				// Since they are raw bytes and not just a string, we need to manually null-terminate it so we can safely interpret it as a string
				// TODO: verify that this is actually required, buffer might just be nulls already
				buffer[readBytes] = '\0';
				if (readBytes > 0) {
					// For testing purposes, print out the (partial) message we received
					std::cout << "Client with FD " << clientFd << " sent:\n" << buffer << "\n";
				}
				// TODO: One of our next goals is to now store these (partial) messages and have something parse them
				// Right now we just print it and forget it existed, which is not ideal
			}
			else if (events[i].events & EPOLLOUT) {
				// existing connection is done sending us data and is now ready to receive a response
				// so here we send our response after we built one using send() (it's a HTTP response)
				// TODO: Maybe first respond with a simple test message to see if it works
				// TODO: but ideally we want to check if a HTTP response is ready for this socket, and if so, retreive that and send it
			}
		}
	}
    // closing the socket.
    close(serverSocket);

    return 0;
}