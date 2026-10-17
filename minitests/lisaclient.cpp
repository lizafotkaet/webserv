#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main()
{
    // creating socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    // specifying address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9898);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // sending connection request
    connect(clientSocket, (struct sockaddr*)&serverAddress,
            sizeof(serverAddress));

    // sending data
    const char* message = "Hello, server! I'm another server";
    send(clientSocket, message, strlen(message), 0);
  char buffer[1024] = {0};
  // recv(clientSocket, buffer, sizeof(buffer), 0);
  // std::cout << buffer << "\n";
    // closing socket
    close(clientSocket);

    return 0;
}