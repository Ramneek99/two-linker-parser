#include <iostream>
#include <fstream>
#include <string.h>
#include <istream>
#include <ctype.h>
#include <map>

using namespace std;

//TO-DO
//symbol too long
//calculate E
//Memory Map needs to be 000
//Last Line

fstream InputFile;
ofstream outputFile;
map <string , int> symbol;
char tempStr[1024];
bool isFirstCall = true, symIsFirstCall = true, errorDetected = true, symIsFirstCall2 = true, newLine = true;
int LineCount = 0, pass1error = 0, offset = 0;
char *strPointer;
int symInt[1024];
char symChar[1024];
string line;

void readLine(fstream *InputFile, string *line, int *LineCount) {
    fill_n(tempStr, sizeof tempStr, 0);
    getline(*InputFile, *line);
    while (line->empty()) {
        if (InputFile->eof()){
            fill_n(tempStr, sizeof tempStr, 0);
            return;
        }
        getline(*InputFile, *line);
        (*LineCount)++;
    }
    line->copy(tempStr, line->size());
    //isFirstCall = true;
    (*LineCount)++;
}

char *getToken() {
    while (1) {
        if (newLine == true) {
            readLine(&InputFile, &line, &LineCount);
            if (strlen(tempStr)==0) {
                return "";
            }
            strPointer = strtok(tempStr, " \t\n");
            offset = strPointer - tempStr + 1;
            if (strPointer == NULL) {
                newLine = true;
            }
            else {
                newLine = false;
                return strPointer;
            }

        } else {
            strPointer = strtok(NULL, " \t\n");
            offset = strPointer - tempStr + 1;
            if (strPointer != NULL) { // found a token
                return strPointer;
            }
            else {
                newLine = true;
            }
        }
    }

}

void __parseerror(int errcode) {
    ofstream outputFile("output.txt", ofstream::trunc);
    string errstr[] = {
            "NUM_EXPECTED", // Number expect, anything >= 2^30 is not a number either
            "SYM_EXPECTED", // Symbol Expected
            "ADDR_EXPECTED", // Addressing Expected which is A/E/I/R
            "SYM_TOO_LONG", // Symbol Name is too long
            "TOO_MANY_DEF_IN_MODULE", // > 16
            "TOO_MANY_USE_IN_MODULE", // > 16
            "TOO_MANY_INSTR", //total num_instr exceeds memory size (512)
    };
    outputFile << "Parse Error line " << LineCount << " offset " << offset << ": " << errstr[errcode];
    outputFile.close();
}

void __parseerror2(int errcode) {
    ofstream outputFile("output.txt", ofstream::trunc);
    string errstr[] = {
            "Error: Absolute address exceeds machine size; zero used",
            "Error: Relative address exceeds module size; zero used",
            "Error: External address exceeds length of uselist; treated as immediate",
            "Error: This variable is multiple times defined; first value used",
            "Error: Illegal immediate value; treated as 9999",
            "Error: Illegal opcode; treated as 9999"
    };
    if (errcode==6){
        outputFile << "Error: %s is not defined; zero used";
    } else if (errcode==7){
        outputFile << "Warning: Module %d: %s too big %d (max=%d) assume zero relative\n";
    } else if (errcode==8){
        outputFile << "Warning: Module %d: %s appeared in the uselist but was not actually used\n";
    } else if (errcode==9){
        outputFile << "Warning: Module %d: %s was defined but never used\n";
    }
    else {
        outputFile << errstr[errcode];
    }
    outputFile.close();
}

int readInt() {
    char *tok = getToken();
    if (strlen(tok)==0){
        return -1;
    }
    if (!isdigit(*tok)) {
       pass1error = 0;
       __parseerror(pass1error);
       exit(-1);
    }
    int value = atoi(tok);
    return value;
}

char *readSym() {
    char *tok = getToken();
//    if (strlen(tok)==0){
//        exit(0);
//    }
    // verify that the token is [a-Z][a-Z0-9]* and no longer than 16
    if (!isalpha(*tok)) {
        pass1error = 1;
        __parseerror(pass1error);
        exit(-1);
    }
    return tok;
}

char readAEIR() {
    char *tok = getToken();
//    if (strlen(tok)==0){
//        exit(0);
//    }
    // make sure only one character and its part 'A','E','I' or 'R'
    if (!isalnum(*tok)) {
        pass1error = 2;
        exit(-1);
    }
    return *tok;
}

void  createSymbol() {
    outputFile.open("output.txt", ofstream::out | ofstream::app);
    cout << "Symbol Table\n";
    outputFile << "Symbol Table\n";
    for (const auto& [key, value]: symbol){
        cout << key << "=" << value << endl;
        outputFile << key << "=" << value << endl;
    }
    //symbol.insert(pair<char*,int>(sym,value));
//    if (symIsFirstCall == true) {
//        outputFile << "Symbol Table\n";
//        cout << "Symbol Table\n";
//        symbol.insert(pair<char*,int>(sym,value));
//        outputFile << sym << "=" << value << "\n";
//        cout << sym << "=" << value << "\n";
//        symIsFirstCall = false;
//    } else {
//        symbol.insert(pair<char*,int>(sym,value));
//        outputFile << sym << "=" << value << "\n";
//        cout << sym << "=" << value << "\n";
//    }
    outputFile.close();
}

void memoryMap(int count, int operand) {
    outputFile.open("output.txt", ofstream::out | ofstream::app);
    if (symIsFirstCall2) {
        outputFile << "\nMemory Map\n";
        outputFile << count << ":" << operand << "\n";
        cout << "\nMemory Map\n";
        cout << count << ":" << operand << "\n";
        symIsFirstCall2 = false;
    } else {
        outputFile << count << ":" << operand << "\n";
        cout << count << ":" << operand << "\n";
    }
    outputFile.close();
}

void clearUseList(string (*uselist)[20]){
    for(auto& sym: *uselist){
        sym.clear();
    }
}

void Pass2() {
    string uselist[20];
    InputFile.open("/Users/rimmyaulakh/CLionProjects/lab1OS/input.txt", ios::in);
    string line;
    int value = 0, count = 000;
    while (!InputFile.eof()) {
        int defcount = readInt();
        for (int i = 0; i < defcount; i++) {
            char *sym = readSym();
            int val = readInt();
        }

        clearUseList(&uselist);
        int usecount = readInt();
        for (int i = 0; i < usecount; i++) {
            char *sym = readSym();
            uselist[i] = string(sym);
        }

        int instcount = readInt();
        for (int i = 0; i < instcount; i++) {
            char addressmode = readAEIR();
            int operand = readInt();
            if (addressmode == 'R') {
                memoryMap(count, operand + value);
                count++;
            } else if (addressmode == 'E') {
                int remainder = operand%1000;
                string searchSymbol = uselist[remainder];
                int valueOperand = (operand - remainder) + symbol[searchSymbol];
//                int remainder = (address)%1000;
//                string AssSym = usearr[remainder];
//                int fa = (address - remainder) + symtable[AssSym];
//                mmaddress = fa;
                memoryMap(count, valueOperand);
                count++;
            } else if (addressmode == 'A') {
                int remainder = operand - operand%1000;
                if(remainder>511){

                }
                memoryMap(count, operand);
                count++;
            } else {
                memoryMap(count, operand);
                count++;
            }
        }
        value = value + instcount;
    }
    InputFile.close();
}

void Pass1() {
    InputFile.open("/Users/rimmyaulakh/CLionProjects/lab1OS/input.txt", ios::in);
    int value = 0, tempValue;
    while (!InputFile.eof()) {
        int defcount = readInt();
        if (defcount == -1){
            continue;
        }
        if (defcount > 16) {
            pass1error = 4;
            __parseerror(pass1error);
            break;
        }
        for (int i = 0; i < defcount; i++) {
            char *sym = readSym();
            int val = readInt();
            value = val + value;
            string key = string(sym);
            symbol[key] = value;
            //createSymbol(sym, value);
            value = value - val;
        }
        int usecount = readInt();
        if (usecount == -1){
            continue;
        }
        if (usecount > 16) {
            pass1error = 5;
            break;
        }
        for (int i = 0; i < usecount; i++) {
            char *sym = readSym();
        }
        int instcount = readInt();
        if (instcount == -1){
            continue;
        }
        if (instcount > 512) {
            pass1error = 6;
            break;
        }
        value = value + instcount;
        for (int i = 0; i < instcount; i++) {
            char addressmode = readAEIR();
            int operand = readInt();
        }
        offset = 0;
    }
    InputFile.close();
    createSymbol();
    Pass2();

}

int main() {
    //readFile();
    Pass1();
}

