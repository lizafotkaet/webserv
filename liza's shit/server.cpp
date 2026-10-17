#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <vector>
#include <array>

using namespace std;

int main()
{
    // creating socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // specifying the address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(6969);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // binding socket.
    bind(serverSocket, (struct sockaddr*)&serverAddress,
         sizeof(serverAddress));

	cout << "Socket bound...\n\n";

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

    // closing the socket.
    close(serverSocket);

    return 0;
}
