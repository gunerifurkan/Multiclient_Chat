#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <thread>

#define PORT 2023

using namespace std;

char buffer[1024];
string input, userName;

void LineGetter(int sock) {
    while (true) {
        getline(cin, input);
        input = userName + ": " + input;

        int sendRes = send(sock, input.c_str(), input.size() + 1, 0);
        if (sendRes == -1) {
            cout << "Could not send to server! Whoops!" << endl;
            continue;
        }
    }
}

void startClient() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\\n Socket creation error \\n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        cout << "\\nInvalid address/ Address not supported \\n";
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        cout << "\\nConnection Failed \\n";
        return;
    }

    cout << "Enter your name: ";
    getline(cin, userName);

    thread senderThread(LineGetter, sock);

    while (true) {
        memset(buffer, 0, 1024);
        int read = recv(sock, buffer, 1024, 0);
        if (read > 0) {
            cout << buffer << endl;
        }
    }
}

int main() {
    startClient();
    return 0;
}
