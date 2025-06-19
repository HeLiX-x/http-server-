#include <iostream>
#include <winsock2.h> // Use winsock2.h instead of winsock.h
#include <ws2tcpip.h> // For modern socket functions
#include <windows.h>  // For Sleep()

#pragma comment(lib, "ws2_32.lib") // Link with Winsock library

using namespace std;

#define PORT 9909
#define MAX_CLIENTS 5

struct sockaddr_in srv;
fd_set fr, fw, fe;
int sockfd;
int arrClientSocket[MAX_CLIENTS] = {0}; // Initialize all to zero

void processNewMessage(int clientSocket) {
    char buffer[257] = {0};
    int nRet = recv(clientSocket, buffer, 256, 0);

    if(nRet <= 0) {
        cout << "Client disconnected on socket: " << clientSocket << endl;
        closesocket(clientSocket);
        for(int i = 0; i < MAX_CLIENTS; i++) {
            if(arrClientSocket[i] == clientSocket) {
                arrClientSocket[i] = 0; // Remove client
                break;
            }
        }
    } else {
        buffer[nRet] = '\0'; // Null-terminate
        cout << "Received message from client on socket " << clientSocket << ": " << buffer << endl;
        send(clientSocket, buffer, nRet, 0); // Echo back
    }
}

void processTheNewRequest() {
    FD_ZERO(&fr);
    FD_ZERO(&fw);
    FD_ZERO(&fe);

    FD_SET(sockfd, &fr);
    FD_SET(sockfd, &fe);

    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(arrClientSocket[i] != 0)
            FD_SET(arrClientSocket[i], &fr);
    }

    timeval tv = {0, 1000}; // timeout
    int maxFd = sockfd;

    int nRet = select(maxFd + 1, &fr, &fw, &fe, &tv);

    if(nRet > 0) {
        // New connection request
        if(FD_ISSET(sockfd, &fr)) {
            struct sockaddr_in clientAddr;
            int addrLen = sizeof(clientAddr);
            int clientSocket = accept(sockfd, (sockaddr*)&clientAddr, &addrLen);

            if(clientSocket != INVALID_SOCKET) {
                bool added = false;
                for(int i = 0; i < MAX_CLIENTS; i++) {
                    if(arrClientSocket[i] == 0) {
                        arrClientSocket[i] = clientSocket;
                        cout << "New client connected on socket: " << clientSocket << endl;
                        send(clientSocket, "Welcome to the server!", 23, 0);
                        added = true;
                        break;
                    }
                }
                if (!added) {
                    cout << "No more space for new clients." << endl;
                    send(clientSocket, "Server full", 12, 0);
                    closesocket(clientSocket);
                }
            }
        }

        // Existing client messages
        for(int i = 0; i < MAX_CLIENTS; i++) {
            if(arrClientSocket[i] != 0 && FD_ISSET(arrClientSocket[i], &fr)) {
                processNewMessage(arrClientSocket[i]);
            }
        }
    } else if(nRet == 0) {
        // Timeout
    } else {
        cout << "Select failed with error: " << WSAGetLastError() << endl;
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "WSAStartup failed." << endl;
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd == INVALID_SOCKET) {
        cout << "Socket creation failed." << endl;
        WSACleanup();
        return -1;
    }

    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));

    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = INADDR_ANY;
    ZeroMemory(srv.sin_zero, sizeof(srv.sin_zero));

    if(bind(sockfd, (sockaddr*)&srv, sizeof(srv)) == SOCKET_ERROR) {
        cout << "Bind failed." << endl;
        closesocket(sockfd);
        WSACleanup();
        return -1;
    }

    if(listen(sockfd, SOMAXCONN) == SOCKET_ERROR) {
        cout << "Listen failed." << endl;
        closesocket(sockfd);
        WSACleanup();
        return -1;
    }

    cout << "Listening on port " << PORT << "..." << endl;

    while(true) {
        processTheNewRequest();
        Sleep(100); // Small delay to avoid busy-wait
    }

    // Cleanup (not reached in this example)
    closesocket(sockfd);
    WSACleanup();
    return 0;
}
