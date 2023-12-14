#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include<vector>
#include <winsock2.h>
#include <thread>
#include<string>
#include<nlohmann/json.hpp>
#include <sstream>
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
        std::cout << "Creating socket: " << socket << std::endl;
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

    void reg(SOCKET clientSocket) {
        char buf[100];
        int bytesRead;
        bytesRead = recv(clientSocket, buf, sizeof(buf), 0);
        buf[bytesRead] = '\0';
        std::string clientName = parseJson(buf, 'm'), res = parseJson(buf, 's');
        
        if (res == "server") {
            for (int i = 0; i < clientNames->size(); i++) {
                json jsonMessage = {
                            {"message", clientName}, {"receiver",clientNames->at(i)}, {"sender", std::string("server")} };
                std::string jsonString = jsonMessage.dump();
                send(clientSockets->at(i), jsonString.c_str(), strlen(jsonString.c_str()), 0);

                jsonMessage = {
                            {"message", clientNames->at(i)}, {"receiver", clientName}, {"sender", std::string("server")} };
                jsonString = jsonMessage.dump();
                send(clientSocket, jsonString.c_str(), strlen(jsonString.c_str()), 0);
            }
            clientNames->push_back(clientName);
            std::cout << "add: " << clientName << std::endl;
        }
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
            reg(temp);
            // Створення окремого потоку для обслуговування клієнта
            std::thread clientThread(&server::gettingMessages, this, clientSockets->back());
            clientThread.detach();  // Детачимо, щоб потік відпрацьовував незалежно
            clientThreads.push_back(std::move(clientThread));
        }            
    }

    // Отримання та вивід повідомлень від клієнта
    void gettingMessages(SOCKET clientSocket) {
        int bytesRead;
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


    // Пересилання повідомлення потрібному отрмувачу
    void sendMessage(char received[1000]) {
        int ind = 0;
        std::string receiver = parseJson(received, 'r');
        if (receiver == "server") {
            std::string clientName = parseJson(received, 'm');

            for (int i = 0; i < clientNames->size(); i++) {
                json jsonMessage = {
                            {"message", clientName}, {"receiver",clientNames->at(i)}, {"sender", std::string("server")} };
                std::string jsonString = jsonMessage.dump();
                send(clientSockets->at(i), jsonString.c_str(), strlen(jsonString.c_str()), 0);
            }
            clientNames->push_back(clientName);
        }
        else {
            std::stringstream ss(receiver);
            std::string token;
            std::vector<std::string> tokens;//всі отримувачі

            while (std::getline(ss, token, ' ')) {
                tokens.push_back(token);
            }

            for (int i = 0; i < tokens.size(); i++) {
                receiver = tokens[i];
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
                std::cout << "sended:" << received << std::endl;
            }
        }
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
