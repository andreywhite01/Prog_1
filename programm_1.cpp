#include "programm_1.h"

int main()
{
    ClientPart server;
    int err = server.connectServer("127.0.0.1", 8080);

    if (err != 0)
        return 1;

    Buffer accessingFile;

    thread th1_getReadAndPrecess(readAndPrecessInput, ref(accessingFile), ref(cin));
    thread th2_getSum(postSumOfIntegersFromStr, ref(accessingFile), ref(server));

    th1_getReadAndPrecess.join();
    th2_getSum.join();

    return 0;
}

// Реализация методов класса Buffer и пользовательских функций

void Buffer::writeInBuffer(string text, size_t inputLength) {

    //lock_guard<mutex> lock(mtx); // До конца выполнения функции доступ к переменной isReady имеет только первый поток
    if (text != "exit") {
        unique_lock<mutex> lock(mtx);
    }
    else {
        text = "";
        inputLength = text.size();
    }

    inBuffer.open(fileName, std::ios::out);
    for (unsigned i = 0; i < inputLength; ++i) {
        inBuffer << text[i];
    }
    inBuffer.close();

    isReady = true;
    fileReadyCondition.notify_all();
}
string Buffer::readFromBuffer() {
    string inputText = "";

    //lock_guard<mutex> lock(mtx); // До конца выполнения функции доступ к переменной isReady имеет только второй поток

    fromBuffer.open(fileName, std::ios::in);
    fromBuffer >> inputText;
    isReady = false;
    fromBuffer.close();

    inBuffer.open(fileName, std::ios::out);
    inBuffer.close();

    return inputText;
}

// Точка входа потока №2
void postSumOfIntegersFromStr(Buffer& buffer, ClientPart& server) {
    while (true) {
        unique_lock<mutex> lock(buffer.mtx);
        if (!buffer.isReady)
            buffer.fileReadyCondition.wait(lock);



        string strLine = buffer.readFromBuffer();

        if (strLine == "") {
            break;
        }

        string::iterator it = strLine.begin();

        int sum = isIntegerInChar(*it) ? C_INT(*it) : 0;

        while (++it != strLine.end()) {
            if (isIntegerInChar(*it))
                sum += C_INT(*it);
        }
        cout << "Transformed array: " << strLine << endl;

        stringstream ss;
        ss << to_string(sum);
        vector <char> buf(BUFF_SIZE);
        ss.read(buf.data(), buf.size());

        server.post(buf);
    }
}

// Точка входа потока №1
void readAndPrecessInput(Buffer& buffer, istream& in) {

    while (true) {
        char arrayOfInputSymbols[MAX_INPUT_SIZE + 1];
        size_t inputLength = readInputAndGetAmount(in, arrayOfInputSymbols);

        if (isExitCommand(arrayOfInputSymbols)) {
            buffer.writeInBuffer("exit", 4);
            break;
        }

        if (inputLength == 0) {
            cerr << "Expected non empty input." << endl;
            continue;
        }

        if (inputLength > MAX_INPUT_SIZE) {
            cerr << "Input length is more then " << MAX_INPUT_SIZE << " symbols. Try again." << endl;
            continue;
        }

        if (!areIntegersInChars(arrayOfInputSymbols, inputLength)) {
            cerr << "There are not only integers in input. Try again." << endl;
            continue;
        }

        qsort(arrayOfInputSymbols, inputLength, sizeof(char),
            [](const void* x1, const void* x2) {return (*(char*)x2 - *(char*)x1); });

        string resultLine = replaceEvensToKB(arrayOfInputSymbols, inputLength);
        size_t resultLength = resultLine.length();
        buffer.writeInBuffer(resultLine, resultLength);
    }
}

string replaceEvensToKB(const char* arr, size_t arrLength) {
    string resultLine = "";
    const char* ptrArr = arr;
    while (ptrArr != arr + arrLength) {
        if ((*ptrArr) % 2 == 0)
            resultLine += "KB";
        else
            resultLine += (*ptrArr);
        ptrArr++;
    }
    return resultLine;
}

unsigned readInputAndGetAmount(istream& in, char* buffer) {
    unsigned index = 0;
    char symbol;
    while (in.get(symbol) && symbol != '\0' && symbol != '\n' && symbol != cin.eof()) {
        writeSymbolInBuffer(symbol, index, buffer);
        index++;
        if (index == MAX_INPUT_SIZE + 1) {
            in.ignore(INT_MAX, '\n');
            in.clear();
            break;
        }
    }
    return index;
}

void writeSymbolInBuffer(char symbol, unsigned index, char* buffer) {

    buffer[index] = symbol;
    buffer[index + 1] = '\0';
}

bool areIntegersInChars(const char* arr, size_t arrLength) {
    for (const char* symbolPtr = arr; symbolPtr < arr + arrLength; symbolPtr++)
        if (!isIntegerInChar(*symbolPtr))
            return false;
    return true;
}

bool isIntegerInChar(char symbol) {
    return symbol >= '0' && symbol <= '9';
}

bool isExitCommand(const char* command) {
    if (strlen(command) == 4) {
        for (unsigned i = 0; i < 4; ++i)
            if (command[i] != EXIT_COMMAND[i]) {
                return false;
            }
        return true;
    }
    return false;
}