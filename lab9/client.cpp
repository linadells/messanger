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
    std::string name;
    int port;
    SOCKET clientSocket;
    const char* serverIP;
    WSADATA wsaData;
    sockaddr_in serverAddr;
    std::vector<std::string> usersNames;

public:
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

    QString receiveMessages() {
        char get[1000];
        int bytesRead;

        while (true) {
            bytesRead = recv(clientSocket, get, sizeof(get) - 1, 0);
            get[bytesRead] = '\0';
            std::cout<<"Get: "<<get<<std::endl;
            std::string sender = parseJson(get, 's'), message = parseJson(get, 'm');

            if (bytesRead > 0) {
                {
                    if(sender=="server"){
                        std::cout<<"im message from server\n";
                        usersNames.push_back(message);
                        std::cout<<"new client: "<<message<<std::endl;

                        std::cout<<"all:\n";
                        for(int i=0;i<usersNames.size();i++){
                            std::cout<<usersNames[i]<<std::endl;
                        }
                    }
                    else{
                    std::cout << sender << ": " << message << std::endl;
                    return QString::fromStdString(sender)+QString::fromStdString(": ")+QString::fromStdString(message);
                    }
                }
            }
            else if (bytesRead == 0) {
                std::cout << "Server is off. End of chat " << std::endl;
                break;
                //return "";
            }
            else {
                std::cerr << "Error of receiving data: " << WSAGetLastError() << std::endl;
                break;
                //return "";
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

    std::vector<std::string>getUsers(){
        return usersNames;
    }
    // Закриття сокету та очищення Winsock
    ~client() {
        closesocket(clientSocket);
        WSACleanup();
    }
};

/*int main() {
    std::cout << "Enter your name:";
    char str[100];
    std::cin.getline(str, sizeof(str));
    client* user = new client(str, 12345, "192.168.1.103");
    user->creatingSocket();
    user->initializeAddrAndConnect();
    std::thread clientThread(&client::receiveMessages, user);
    clientThread.detach();  // Детачимо, щоб потік відпрацьовував незалежно
    user->sendMessages();
    return 0;
} */
