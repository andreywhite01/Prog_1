#include "programm_1.h"

int main()
{
    ClientPart server;

    int err = server.connectToServer("127.0.0.1", 8080);
    cout << "\t\tConnection successfully established." << endl;
    Buffer accessingFile;

    thread th1_getReadAndPrecess(readAndPrecessInput, ref(accessingFile), ref(cin));
    thread th2_getSum(postSumOfIntegersFromString, ref(accessingFile), ref(server));

    th1_getReadAndPrecess.join();
    th2_getSum.join();

    return 0;
}

// Реализация методов класса Buffer и пользовательских функций

void Buffer::writeInBuffer(string text, size_t inputLength) {

    //lock_guard<mutex> lock(mtx); // До конца выполнения функции доступ к переменной isReady имеет только первый поток

    unique_lock<mutex> lock(mtx);

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

    fromBuffer.open(fileName, std::ios::in);
    fromBuffer >> inputText;
    isReady = false;
    fromBuffer.close();

    inBuffer.open(fileName, std::ios::out);
    inBuffer.close();

    return inputText;
}
void Buffer::deleteBuffer() {
    remove(fileName);
}

// Точка входа потока №2
void postSumOfIntegersFromString(Buffer& buffer, ClientPart& server) {
    while (true) {

        string strLine = "";
        {
            unique_lock<mutex> lock(buffer.mtx);
            if (!buffer.isReady)
                buffer.fileReadyCondition.wait(lock);
            strLine = buffer.readFromBuffer();
        }

        // Проверяем соединение с сервером, 
        // если соединение отсутствует, то пробуем переподключиться
        server.post({ TEST_MESSAGE });
        if (server.getConnectionStatus() == false) {
            cout << "\n\t\tConnection lost.\n\t\tLast value was not transferred to programm 2.\n\t\tTrying to reconnect..." << endl;
            server.reconnect();
            cout << "\n\t\tConnection restored" << endl;
            buffer.deleteBuffer();
            buffer.isReady = false;
            continue;
        }
        cout << "Transformed array: " << strLine << endl;

        vector <char> buf(BUFF_SIZE);

        if (strLine != EXIT_COMMAND) {
            int sum = getSumOfIntegersFromString(strLine);

            stringstream ss;
            ss << to_string(sum);
            ss.read(buf.data(), buf.size());
        }
        if (server.getConnectionStatus() == true)
            server.post(buf);
    }
}

// Точка входа потока №1
void readAndPrecessInput(Buffer& buffer, istream& in) {

    while (true) {
        char arrayOfInputSymbols[MAX_INPUT_SIZE + 1];
        size_t inputLength = readInputAndGetAmount(in, arrayOfInputSymbols);

        if (isExitCommand(arrayOfInputSymbols)) {
            buffer.writeInBuffer(EXIT_COMMAND, 4);
            break;
        }

        if (isInputLineInCorrectFormat(arrayOfInputSymbols, inputLength)) {
            qsort(arrayOfInputSymbols, inputLength, sizeof(char),
                [](const void* x1, const void* x2) {return (*(char*)x2 - *(char*)x1); });

            string resultLine = replaceEvensToKB(arrayOfInputSymbols, inputLength);
            size_t resultLength = resultLine.length();
            buffer.writeInBuffer(resultLine, resultLength);
        }
    }
}

bool isInputLineInCorrectFormat(const char* arrayOfInputSymbols, size_t inputLength) {
    if (inputLength == 0) {
        cout << "Expected non empty input." << endl;
        return false;
    }
    if (inputLength > MAX_INPUT_SIZE) {
        cout << "Input length is more then " << MAX_INPUT_SIZE << " symbols. Try again." << endl;
        return false;
    }
    if (!areIntegersInChars(arrayOfInputSymbols, inputLength)) {
        cout << "There are not only integers in input. Try again." << endl;
        return false;
    }
    return true;
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
        writeSymbolInBufferIndex(symbol, index, buffer);
        index++;
        if (index == MAX_INPUT_SIZE + 1) {
            in.ignore(INT_MAX, '\n');
            in.clear();
            break;
        }
    }
    return index;
}

void writeSymbolInBufferIndex(char symbol, unsigned index, char* buffer) {

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

int getSumOfIntegersFromString(string strLine) {
    string::iterator it = strLine.begin();

    int sum = isIntegerInChar(*it) ? C_INT(*it) : 0;

    while (++it != strLine.end()) {
        if (isIntegerInChar(*it))
            sum += C_INT(*it);
    }
    return sum;
}