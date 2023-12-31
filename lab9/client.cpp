#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include<string>
#include<thread>
#include<nlohmann/json.hpp>
#include<QString>
#include<vector>

using json = nlohmann::json;
#pragma comment(lib, "ws2_32.lib")

class client {
    //Q_OBJECT
public:
    std::string name;
    int port;
    SOCKET clientSocket;
    const char* serverIP;
    WSADATA wsaData;
    sockaddr_in serverAddr;
    std::vector<std::string> usersNames;

    client(std::string name, int port, const char* serverIP) {
        this->name = name;
        this->port = port;
        this->serverIP = serverIP;

        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "Error initializing Winsock " << std::endl;
        }
    }
    int creatingSocket() {
        // Створення сокету
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
            WSACleanup();
            return 1;
        }
    }
    int initializeAddrAndConnect() {
        // Налаштування адреси сервера
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr(serverIP/*"192.168.1.93"*/);
        serverAddr.sin_port = htons(port);

        // Підключення до сервера
        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Error connecting to server: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }
        sendMessages(QString::fromStdString(name), QString::fromStdString("server"), QString::fromStdString(name));
        std::cout << "Connected to server. You are able to start a chat" << std::endl;
    }

    int sendMessages(QString message, QString name, QString receiver) {
            json jsonMessage = {
                {"message", message.toStdString()}, {"receiver", receiver.toStdString()}, {"sender", name.toStdString()} };
            std::string jsonString = jsonMessage.dump();
            send(clientSocket, jsonString.c_str(), strlen(jsonString.c_str()), 0);
            return 0;
    }
    // Закриття сокету та очищення Winsock
    ~client() {
        closesocket(clientSocket);
        WSACleanup();
    }
};

