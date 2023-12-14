#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include<vector>
#include <winsock2.h>
#include <thread>
#include<string>
#include<nlohmann/json.hpp>
using json = nlohmann::json;

class server {
    SOCKET serverSocket;
    std::vector<SOCKET> *clientSockets = new std::vector<SOCKET>;
    std::vector<std::string>* clientNames = new std::vector<std::string>;
    sockaddr_in serverAddr;
    WSADATA wsaData;
    char buffer[1000];
    std::vector<std::thread> clientThreads;
public:
    //в конструкторі ініціалізується бібліотека Winsock
    server() {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "Error initializing winsock" << std::endl;
        }
    }
    std::vector<SOCKET>* getClients() {
        return clientSockets;
    }
    int createSocket() {
        // Створення сокету
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "Error of creating socket: " << WSAGetLastError() << std::endl;
            WSACleanup();
            return 1;
        }
        return 0;
    }
    // Налаштування адреси сервера та прив'язка сокету до адреси
    int initializeAddr(int port) {
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);  // Замініть на потрібний порт

        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Error of connecting socket to addres: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }
        std::cout << "Server is created. Waiting..." << std::endl;
        return 0;
    }

    // Очікування підключення клієнта
    int waitingClients() {
        // Прослуховування вхідних підключень
        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "Error start listerning: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }
        while (true) {
            SOCKET temp = accept(serverSocket, NULL, NULL);
            if (temp == INVALID_SOCKET) {
                std::cerr << "Error of conecting clients " << WSAGetLastError() << std::endl;
                closesocket(serverSocket);
                WSACleanup();
                return 1;
            }
            clientSockets->push_back(temp);
            std::cout << "Client is ready. You are able to start a chat" << std::endl;
            // Створення окремого потоку для обслуговування клієнта
            std::thread clientThread(&server::gettingMessages, this, clientSockets->back());
            clientThread.detach();  // Детачимо, щоб потік відпрацьовував незалежно
            clientThreads.push_back(std::move(clientThread));
        }            
    }

    // Отримання та вивід повідомлень від клієнта
    void gettingMessages(SOCKET clientSocket) {
        int bytesRead;
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        std::string clientName = std::string(buffer);
        clientNames->push_back(clientName);
        while (true) {
            bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';  // додаємо нуль-термінатор
                std::cout << "Client: " << buffer << std::endl;
                sendMessage(buffer);
            }
            else if (bytesRead == 0) {
                std::cout << "Client is off" << std::endl;
                break;
            }
            else {
                std::cerr << "Error getting messages from client: " << WSAGetLastError() << std::endl;
                break;
            }
        }
        closesocket(clientSocket);
    }

    // Відправка відповіді клієнту
    void sendMessage(SOCKET clientSocket) {
        std::cout << "Enter message to client:";
        std::cin.getline(buffer, sizeof(buffer));
        send(clientSocket, buffer, strlen(buffer), 0);
    }

    // Пересилання повідомлення потрібному отрмувачу
    void sendMessage(char received[1000]) {
        int ind = 0;
        std::string receiver = parseJson(received, 'r');
        for (auto it = clientNames->begin(); it != clientNames->end(); it++) {
            if ((*it) == receiver) {
                break;
            }
            ind++;
        }
        if (ind == clientNames->size()) {
            std::cerr << "There aren`t such receivers ";
            return;
        }
        SOCKET clientSocket = clientSockets->at(ind);
        send(clientSocket, received, strlen(received), 0);
    }
    std::string parseJson(const char* jsonString, char t) {
        try {
            json jsonObject = json::parse(jsonString);
            std::string res;
            if (t == 'm') {
                res = jsonObject["message"].get<std::string>();
            }
            else if (t == 'r') {
                res = jsonObject["receiver"].get<std::string>();
            }
            else if (t == 's') {
                res = jsonObject["sender"].get<std::string>();
            }
            return res;
        }
        catch (const std::exception& e) {
            std::cerr << "Error parsing JSON";
        }
    }

    // Закриття сокетів та очищення Winsock
    ~server() {
        for (int i = 0; i < clientSockets->size(); i++) {
            closesocket((*clientSockets)[i]);
        }
        closesocket(serverSocket);
        WSACleanup();
    }
};
int main() {
    server* serv = new server();
    serv->createSocket();
    serv->initializeAddr(12345);
    serv->waitingClients();
    return 0;
}
