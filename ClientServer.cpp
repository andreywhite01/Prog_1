#include "ClientServer.h"

//Реализация методов базового класса ClientServer
ClientServer::ClientServer() {
    tryWSAStartup();
    tryInitializeSocket();
}
ClientServer::~ClientServer() {
    closesocket(Sock);
    WSACleanup();
}
int ClientServer::tryWSAStartup() {
    err = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (err != 0) {
        cerr << "Error WinSock version initializaion #";
        cerr << WSAGetLastError();
        return 1;
    }
    else
        cerr << "WinSock initialization is OK" << endl;
    return 0;
}
int ClientServer::setServInfo(const char* ip, unsigned short port) {
    ZeroMemory(&servInfo, sizeof(servInfo));

    in_addr ip_to_num;
    err = inet_pton(iFamily, ip, &ip_to_num);
    if (err <= 0) {
        cerr << "Error in IP translation to special numeric format" << endl;
        return 1;
    }
    else {
        cerr << "IP translation to special numeric format is OK" << endl;
    }

    servInfo.sin_family = iFamily;
    servInfo.sin_addr = ip_to_num;
    servInfo.sin_port = htons(port);
    return 0;
}
int ClientServer::tryInitializeSocket() {
    Sock = socket(iFamily, SOCK_STREAM, 0);
    if (Sock == INVALID_SOCKET) {
        cerr << "Error initialization socket # " << WSAGetLastError() << endl;
        closesocket(Sock);
        WSACleanup();
        return 1;
    }
    else
        cerr << "Client socket initialization is OK" << endl;
    return 0;
}

//Реализация методов класса, унаследованного от базового ClientPart

void ClientPart::connectToServer(const char* ip, unsigned short port) {
    setServInfo(ip, port);
    tryConnectToServer();
}
void ClientPart::post(const vector <char>& buf) {
    short packet_size = send(Sock, buf.data(), (int)buf.size(), 0);
}

int ClientPart::tryConnectToServer() {
    err = connect(Sock, (sockaddr*)&servInfo, sizeof(servInfo));

    if (err != 0) {
        cerr << "Connection to Server is FAILED. Error # " << WSAGetLastError() << endl;
        closesocket(Sock);
        WSACleanup();
        return 1;
    }
    else
        cerr << "Connection established SUCCESSFULLY. Ready to send a message to Server" << endl;
    return 0;
}

//Реализация методов класса, унаследованного от базового Server

void ServerPart::createServer(const char* ip, unsigned short port) {
    setServInfo(ip, port);
    tryServerBinding();
}
void ServerPart::createConnection() {
    tryListening();
    connectToClient();
}
int ServerPart::reconnect() {
    sockaddr_in clientInfo;
    ZeroMemory(&clientInfo, sizeof(clientInfo));
    int clientInfo_size = sizeof(clientInfo);
    ClientConn = accept(Sock, (sockaddr*)&clientInfo, &clientInfo_size);
    if (ClientConn == INVALID_SOCKET) {
        cerr << "Client detected, but can't connect to a client. Error # " << WSAGetLastError() << endl;
        closesocket(Sock);
        closesocket(ClientConn);
        WSACleanup();
        return 1;
    }
    else
        cerr << "Reconncetion to a client established successfully" << endl;
    return 0;
}
SOCKET ServerPart::getServSock() {
    return Sock;
}
SOCKET ServerPart::getClientConn() {
    return ClientConn;
}
int ServerPart::tryServerBinding() {
    err = bind(Sock, (SOCKADDR*)&servInfo, sizeof(servInfo));

    if (err != 0) {
        cerr << "Error socket binding to server info # ";
        cerr << WSAGetLastError() << endl;
        closesocket(Sock);
        WSACleanup();
        return 1;
    }
    else
        cerr << "Binding socket to Server info is OK" << endl;
    return 0;
}
int ServerPart::tryListening() {
    err = listen(Sock, SOMAXCONN);

    if (err != 0) {
        cerr << "Can't start to listen to # ";
        cerr << WSAGetLastError() << endl;
        closesocket(Sock);
        WSACleanup();
        return 1;
    }
    else {
        cerr << "Listening..." << endl;
    }
    return 0;
}
int ServerPart::connectToClient() {
    sockaddr_in clientInfo;

    ZeroMemory(&clientInfo, sizeof(clientInfo));

    int clientInfo_size = sizeof(clientInfo);

    ClientConn = accept(Sock, (sockaddr*)&clientInfo, &clientInfo_size);

    if (ClientConn == INVALID_SOCKET) {
        cerr << "Client detected, but can't connect to a client. Error # " << WSAGetLastError() << endl;
        closesocket(Sock);
        closesocket(ClientConn);
        WSACleanup();
        return 1;
    }
    else
        cerr << "Connection to a client established successfully" << endl;
    return 0;
}