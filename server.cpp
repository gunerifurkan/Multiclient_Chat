#include <iostream>
#include <vector>
#include <thread>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 2023
#define MAX_CLIENTS 7

using namespace std;

map<int, pair<string, thread>> clientThreads;

void handleClient(int clientSocket) {
    char buffer[1024];

    while (true) {
        memset(buffer, 0, 1024);
        int read = recv(clientSocket, buffer, 1024, 0);

        if (read <= 0) {
            cout << "Client disconnected or error" << endl;
            close(clientSocket);
            clientThreads.erase(clientSocket);
            break;
        }

        for (auto const& pair : clientThreads) {
            if (pair.first != clientSocket) {
                send(pair.first, buffer, strlen(buffer), 0);
            }
        }
    }
}

void startServer() {
    int serverFd, newSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(serverFd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (true) {
        if ((newSocket = accept(serverFd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        cout << "New connection: " << newSocket << endl;

        clientThreads[newSocket] = make_pair("", thread(handleClient, newSocket));
    }
}

int main() {
    startServer();
    return 0;
}
