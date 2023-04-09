//  _    _ _______ __  __ _      ____            _   __          __  _     _____                          
// | |  | |__   __|  \/  | |    |  _ \          (_)  \ \        / / | |   / ____|                         
// | |__| |  | |  | \  / | |    | |_) | __ _ ___ _  __\ \  /\  / /__| |__| (___   ___ _ ____   _____ _ __ 
// |  __  |  | |  | |\/| | |    |  _ < / _` / __| |/ __\ \/  \/ / _ \ '_ \\___ \ / _ \ '__\ \ / / _ \ '__|
// | |  | |  | |  | |  | | |____| |_) | (_| \__ \ | (__ \  /\  /  __/ |_) |___) |  __/ |   \ V /  __/ |   
// |_|  |_|  |_|  |_|  |_|______|____/ \__,_|___/_|\___| \/  \/ \___|_.__/_____/ \___|_|    \_/ \___|_|   
                                                                                                        
                                                                                                        

//HTMLBasicWebServer
// (C) Pablo 2023



#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup fallido: " << iResult << std::endl;
        return 1;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Error al crear el socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in localAddress;
    localAddress.sin_family = AF_INET;
    localAddress.sin_addr.s_addr = INADDR_ANY;
    localAddress.sin_port = htons(8881);

    iResult = bind(listenSocket, (sockaddr*)&localAddress, sizeof(localAddress));
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Error al asociar el socket: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Error al escuchar el socket: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Servidor web local iniciado. Acceda a 127.0.0.1:8881 en su navegador." << std::endl;

    while (true) {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error al aceptar la conexión entrante: " << WSAGetLastError() << std::endl;
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }

        char buffer[4096];
        iResult = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (iResult == SOCKET_ERROR) {
            std::cerr << "Error al recibir los datos del cliente: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            continue;
        }

        std::stringstream ss(buffer);
        std::string requestType, path, httpVersion;
        ss >> requestType >> path >> httpVersion;

        if (requestType == "GET") {
            if (path == "/") {
                path = "/index.html";
            }
            std::string filePath = "." + path;
            std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
            if (!file.is_open()) {
                std::cerr << "Error al abrir el archivo solicitado: " << path << std::endl;
                std::ostringstream responseStream;
                responseStream << "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
                std::string response = responseStream.str();
                send(clientSocket, response.c_str(), response.size(), 0);
                closesocket(clientSocket);
                continue;
            }

            std::streampos fileSize = file.tellg();
            if (fileSize == -1) {
                std::cerr << "Error al obtener el tamaño del archivo solicitado: " << path << std::endl;
                closesocket(clientSocket);
                file.close();
                continue;
            }

            char* fileContents = new char[fileSize];
            file.seekg(0, std::ios::beg);
            file.read(fileContents, fileSize);
            file.close();

            std::ostringstream responseStream;
            responseStream << "HTTP/1.1 200 OK\r\nContent-Length: " << fileSize << "\r\n\r\n";
            std::string responseHeader = responseStream.str();
            send(clientSocket, responseHeader.c_str(), responseHeader.size(), 0);
            send(clientSocket, fileContents, fileSize, 0);
            delete[] fileContents;
            closesocket(clientSocket);
        }
    }




    WSACleanup();
    return 0;
}
 
