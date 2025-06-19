#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring> // For memset and string functions

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define PORT 9909
#define BUFFER_SIZE 256

int main() {
    WSADATA ws;
    if (WSAStartup(MAKEWORD(2, 2), &ws) != 0) {
        cout << "WSAStartup failed." << endl;
        return EXIT_FAILURE;
    }

    int clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        cout << "Socket creation failed." << endl;
        WSACleanup();
        return EXIT_FAILURE;
    } else {
        cout << "Socket created successfully." << endl;
    }

    sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &srv.sin_addr); // Connect to localhost
    ZeroMemory(srv.sin_zero, sizeof(srv.sin_zero));

    if (connect(clientSocket, (sockaddr*)&srv, sizeof(srv)) == SOCKET_ERROR) {
        cout << "Connection failed." << endl;
        closesocket(clientSocket);
        WSACleanup();
        return EXIT_FAILURE;
    } else {
        cout << "Connected to server on port " << PORT << "." << endl;
    }

    char buffer[BUFFER_SIZE] = {0};

    // Receive initial welcome message
    int nRet = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    if (nRet > 0) {
        buffer[nRet] = '\0';
        cout << "Server says: " << buffer << endl;
    }

    while (true) {
        cout << "Enter message to send (or 'exit' to quit): ";
        fgets(buffer, BUFFER_SIZE, stdin);

        // Remove newline character if present
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "exit") == 0) {
            cout << "Closing connection..." << endl;
            break;
        }

        // Send message to server
        if (send(clientSocket, buffer, strlen(buffer), 0) == SOCKET_ERROR) {
            cout << "Send failed." << endl;
            break;
        }

        cout << "Waiting for response..." << endl;
        nRet = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);

        if (nRet <= 0) {
            cout << "Server disconnected." << endl;
            break;
        }

        buffer[nRet] = '\0'; // Null-terminate received data
        cout << "Echo from server: " << buffer << endl;
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}