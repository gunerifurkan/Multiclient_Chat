#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <clocale>
#include <sys/types.h>
#include <netdb.h>
#include <string>
#include <sstream>
#include <pthread.h>
#include <sys/time.h>

#define MAX_CLIENTS 7

using namespace std;

int clientSocket[MAX_CLIENTS] = {0};
int serverSocket;
struct sockaddr_in addr;
int addrlen = sizeof(addr);

void initServer() {
    setlocale(LC_ALL, "Turkish");

    int opt = 1;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(serverSocket == -1) {
        cerr << "Soket Oluşmadı! << endl;
        exit(EXIT_FAILURE);
    }

    if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
        cerr << "setsockopt" << endl;
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(2023);
    inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr);

    if(bind(serverSocket, (sockaddr*)&addr, sizeof(addr)) < 0) {
        cerr << "Bağlantı Hatası!" << endl;
        exit(EXIT_FAILURE);
    } else {
        cout << "Bağlantı Başarılı..." << endl;
    }

    if(listen(serverSocket, MAX_CLIENTS) < 0) {
        cerr << "Dinleme Hatası!" << endl;
        exit(EXIT_FAILURE);
    } else {
        cout << "Dinleme Başarılı..." << endl;
    }
}

void acceptConnections() {
    int newSocket;
    if((newSocket = accept(serverSocket, (struct sockaddr*)&addr, (socklen_t*)&addrlen)) < 0) {
        cerr << "İstek Kabulü Hatası!" << endl;
        exit(EXIT_FAILURE);
    } else {
        cout << "İstek Kabulü Başarılı... \n\n";
    }

    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clientSocket[i] == 0) {
            clientSocket[i] = newSocket;
            break;
        }
    }
}

void handleClients(fd_set readfds) {
    char buffer[1024];
    int bytesRecieved;

    for(int i = 0; i < MAX_CLIENTS; i++) {
        int sd = clientSocket[i];
        if(FD_ISSET(sd, &readfds)) {
            bytesRecieved = recv(sd, buffer, 1024, 0);

            if(bytesRecieved <= 0) {
                if(bytesRecieved == -1) {
                    cerr << "Veri Alma Hatası!" << endl;
                } else {
                    cout << "Bağlantı Kesildi!" << endl;
                }
                close(sd);
                clientSocket[i] = 0;
            } else {
                int exclude = sd;
                cout << string(buffer, 0, bytesRecieved) << endl;

                ostringstream ss;
                ss << buffer << "\r\n";
                string strOut = ss.str();

                for(int j = 0; j < MAX_CLIENTS; j++) {
                    sd = clientSocket[j];
                    if(sd != serverSocket && sd != 0 && exclude != sd) {
                        send(sd, strOut.c_str(), strOut.size() + 1, 0);
                        FD_CLR(sd, &readfds);
                    }
                }
            }
        }
    }
}

int main() {
    initServer();

    struct timeval interval;
    interval.tv_sec=0.2;
    interval.tv_usec=0;

    fd_set readfds;

    while(1) {
        FD_ZERO(&readfds);

        int max_sd = serverSocket;

        FD_SET(serverSocket, &readfds);
        for(int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clientSocket[i];

            if(sd > 0)
                FD_SET(sd, &readfds);

            if(sd > max_sd)
                max_sd = sd;
        }

        int activity = select(max_sd + 1, &readfds, NULL, NULL, &interval);
        if((activity < 0) && (errno != EINTR)) {
            cout << "Soket Seçme Hatası!" << endl;
        }

        if(FD_ISSET(serverSocket, &readfds)) {
            acceptConnections();
        }

        handleClients(readfds);
    }

    return 0;
}
