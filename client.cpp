#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <clocale>
#include <sys/types.h>
#include <netdb.h>
#include <string>
#include <thread>

using namespace std;

char buffer[1024];
string input,userName;

void LineGetter(int path)
{
    while(1)
    {
        getline(cin, input);
        input = userName + ": " + input;

        int output = send(path, input.c_str(), input.size() + 1, 0);
        if (output == -1)
        {
            cout << "Veri Gönderimi Hatası!" << endl;
        }
    }
}

int main()
{
    setlocale(LC_ALL, "Turkish");

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(2023);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    int path = socket(AF_INET, SOCK_STREAM, 0); // IPv4, TCP

    if(connect (path, (struct sockaddr*) &server, sizeof(server)) < 0) // 0 = CORRECT
    {
        cout << "Sunucu Bağlantısı Hatası!" << endl;
        return 1;
    }
    else
    {
        cout << "Sunucu Bağlantısı Başarılı... \n\n";
    }

    cout << "Kullanıcı Adı: ";
    getline(cin, userName);

    thread th1(LineGetter, path);

    do
    {
        int d_size = recv(path, buffer, 1024, 0);
        if (d_size == -1)
        {
            cout << "Yanıt Hatası!" << endl;
        }

        if(d_size > 0)
        {
            cout << string(buffer, 0, d_size) << endl;
        }
    } while(1);

    return 0;
}
