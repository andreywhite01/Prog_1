#pragma once

#define _CRT_SECURE_NO_WARNINGS 1
#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

#define BUFF_SIZE 5
#define MAX_INPUT_SIZE 64
#define C_INT(x) ((int)((x) - '0'))

using namespace std;

class ClientServer {
public:
    ClientServer();
    ~ClientServer();
protected:
    int err = 0;
    WSADATA wsaData;
    ADDRESS_FAMILY iFamily = AF_INET;
    SOCKADDR_IN servInfo;
    SOCKET Sock;

    int tryWSAStartup();
    int setServInfo(const char* ip, unsigned short port);
    int tryInitializeSocket();
};

class ClientPart :public ClientServer {
public:
    ClientPart() : ClientServer() {};
    int connectServer(const char* ip, unsigned short port);
    void post(const vector<char>& buf);
private:
    int tryConnectToServer();
};

class ServerPart : public ClientServer {
public:
    ServerPart() : ClientServer() {};
    SOCKET getServSock();
    SOCKET getClientConn();
    void createServer(const char* ip, unsigned short port);
    void createConnection();
    int reconnect();
private:
    SOCKET ClientConn;

    int tryServerBinding();
    int tryListening();
    int connectToClient();
};