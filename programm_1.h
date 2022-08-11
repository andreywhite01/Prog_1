#pragma once

#define _CRT_SECURE_NO_WARNINGS 1
#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <sstream>
#include "ClientServer.h"
#include <condition_variable>

#pragma comment(lib, "Ws2_32.lib")

#define BUFF_SIZE 5
#define MAX_INPUT_SIZE 64
#define C_INT(x) ((int)((x) - '0'))
#define EXIT_COMMAND "exit"

using namespace std;

class Buffer {
private:
    ofstream inBuffer;
    ifstream fromBuffer;
    const string fileName = "buffer.txt";
public:
    bool isReady = false;
    mutable mutex mtx;
    condition_variable fileReadyCondition;

    void writeInBuffer(string text, size_t inputLength);
    string readFromBuffer();
};

// �������� �� ������ ���������� ������ ������� ������,
// �� ���� ����� ����� ����� �� ������.
// ����� ����� ������� ������
void postSumOfIntegersFromStr(Buffer& buffer, ClientPart& server);

// ������ ������, ��������� �������������,
// ��������� �� � ������������ � ��������
// � ������ � ����. 
// ����� ����� ������� ������
void readAndPrecessInput(Buffer& buffer, istream& in);

// ��������������� �������
bool isIntegerInChar(char symbol);
bool areIntegersInChars(const char* arr, size_t arrLength);
string replaceEvensToKB(const char* arr, size_t arrLength);
unsigned readInputAndGetAmount(istream& in, char* buffer);
void writeSymbolInBuffer(char symbol, unsigned index, char* buffer);
bool isExitCommand(const char* command);