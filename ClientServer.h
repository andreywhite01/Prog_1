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
    void setConnectionStatus(bool status);
    bool getConnectionStatus();
protected:
    int err = 0;
    const char* ip = "";
    unsigned short port = 0;
    WSADATA wsaData;
    ADDRESS_FAMILY iFamily = AF_INET;
    SOCKADDR_IN servInfo;
    SOCKET Sock;
    bool isConnected = false;

    int tryWSAStartup();
    int setServInfo(const char* ip, unsigned short port);
    int tryInitializeSocket();
    virtual void reconnect() = 0;
};

class ClientPart :public ClientServer {
public:
    ClientPart() : ClientServer() {};
    int connectToServer(const char* ip, unsigned short port);
    short post(const vector<char>& buf);
    void reconnect();
private:
    int tryConnectToServer();
};

class ServerPart :public ClientServer {
public:
    ServerPart() : ClientServer() {};
    SOCKET getServSock();
    SOCKET getClientConn();
    void createServer(const char* ip, unsigned short port);
    void createConnection();
    void reconnect();
private:
    SOCKET ClientConn;

    int tryServerBinding();
    int tryListening();
    int connectToClient();
};