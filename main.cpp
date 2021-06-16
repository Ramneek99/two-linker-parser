#include <iostream>
#include <fstream>
#include <string.h>
#include <istream>
#include <ctype.h>
#include <map>
#include<set>

using namespace std;

//TO-DO
//Last Line offset

fstream InputFile;
ofstream outputFile;
map <string , int> symbol;
map <string , int> symbolSizeCheck;
map <string , int> symbolModule;
map <string, string> symbolError;
set<string> usedSymbols;
char tempStr[1024];
bool isFirstCall = true, symIsFirstCall = true, errorDetected = true, symIsFirstCall2 = true, newLine = true;
int LineCount = 0, pass1error = 0, offset = 0;
char *strPointer;
int symInt[1024];
char symChar[1024];
string file;
string line;
string symbolPass2;
int module=1;

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
                offset++;
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
            if (strPointer != NULL) { // found a token
                offset = strPointer - tempStr + 1;
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
    cout << "Parse Error line " << LineCount << " offset " << offset << ": " << errstr[errcode];
    outputFile.close();
}

void __parseerror2(int errcode) {
    ofstream outputFile("output.txt", ios::out | ios::app);
    outputFile << " ";
    cout << " ";
    string errstr[] = {
            "Error: Absolute address exceeds machine size; zero used",
            "Error: Relative address exceeds module size; zero used",
            "Error: External address exceeds length of uselist; treated as immediate",
            "Error: This variable is multiple times defined; first value used",
            "Error: Illegal immediate value; treated as 9999",
            "Error: Illegal opcode; treated as 9999",
            "Error: %s is not defined; zero used"
    };
    if(errcode<6) {
        outputFile << errstr[errcode];
        cout << errstr[errcode];
    }
    else{
        outputFile << "Error: "<<symbolPass2<<" is not defined; zero used";
        cout  << "Error: "<<symbolPass2<<" is not defined; zero used";
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
    if (!isalpha(*tok)) {
        pass1error = 1;
        __parseerror(pass1error);
        exit(-1);
    }
    return tok;
}

char readAEIR() {
    char *tok = getToken();
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
        outputFile << key << "=" << value;
        cout << key << "=" << value;
        if(symbolError.find(key) != symbolError.end()){
            cout << symbolError[key] << endl;
            outputFile << symbolError[key] << endl;
        }else{
            cout << endl;
            outputFile << endl;
        }

    }
    outputFile.close();
}

void memoryMap(int count, int operand) {
    outputFile.open("output.txt", ofstream::out | ofstream::app);
    if (symIsFirstCall2) {
        outputFile << "\nMemory Map\n";
        outputFile << setfill('0') << std::setw(3) << count;
        outputFile << ":" ;
        outputFile << setfill('0') << std::setw(4) << operand;
        cout << "\nMemory Map\n";
        cout << setfill('0') << std::setw(3) << count;
        cout << ":";
        cout << setfill('0') << std::setw(4) << operand;
        symIsFirstCall2 = false;
    } else {
        outputFile << "\n";
        outputFile << setfill('0') << std::setw(3) << count;
        outputFile << ":" ;
        outputFile << setfill('0') << std::setw(4) << operand;
        cout << "\n";
        cout << setfill('0') << std::setw(3) << count;
        cout << ":";
        cout << setfill('0') << std::setw(4) << operand;
    }
    outputFile.close();
}

void clearUseList(string (*uselist)[20]){
    for(auto& sym: *uselist){
        sym.clear();
    }
}

void checkUnusedSymbols(set<string>* usedList,set<string>* actuallyUsed){
    outputFile.open("output.txt", ofstream::out | ofstream::app);
    for(const auto& symbols : *usedList){
        if (actuallyUsed->find(symbols) == actuallyUsed->end()){
            // usedList does not found in actuallyUsed set.
            char output[100];
            sprintf(output, "Warning: Module %d: %s appeared in the uselist but was not actually used\n", module, symbols.c_str());
            outputFile << output;
            cout << output;
        }
    }

    outputFile.close();
}

void checkUndefinedSymbols(){
    outputFile.open("output.txt", ofstream::out | ofstream::app);
    for (const auto& [key, value]: symbol){
        if (usedSymbols.find(key) == usedSymbols.end()){
            char output[100];
            sprintf(output, "Warning: Module %d: %s was defined but never used\n", symbolModule[key], key.c_str());
            outputFile << output;
            cout << output;
        }
    }
    outputFile.close();
}

void addNewLine(){
    outputFile.open("output.txt", ofstream::out | ofstream::app);
    outputFile << "\n\n";
    outputFile.close();
}

void checkSymbolSize(int instacount, int instructions){
    outputFile.open("output.txt", ofstream::out | ofstream::app);
    for (const auto& [key, value]: symbolSizeCheck){
        if( value > instacount-1){
            char output[100];
            sprintf(output, "Warning: Module %d: %s too big %d (max=%d) assume zero relative\n", module, key.c_str(),value, instacount-1);
            outputFile << output;
            cout << output;
            symbol[key] =instructions;
        }
    }
    symbolSizeCheck.clear();
    outputFile.close();
}

void Pass2() {
    string uselist[20];
    set<string> actuallyUsed;
    InputFile.open(file, ios::in);
    string line;
    int value = 0, count = 000;
    while (!InputFile.eof()) {
        int defcount = readInt();
        for (int i = 0; i < defcount; i++) {
            char *sym = readSym();
            int val = readInt();
        }

        clearUseList(&uselist);
        actuallyUsed.clear();
        int usecount = readInt();
        for (int i = 0; i < usecount; i++) {
            char *sym = readSym();
            uselist[i] = string(sym);
            usedSymbols.insert(string(sym));
        }

        int instcount = readInt();
        for (int i = 0; i < instcount; i++) {
            char addressmode = readAEIR();
            int operand = readInt();
            if (addressmode == 'R') {
                if(operand>9999){
                    memoryMap(count, 9999);
                    __parseerror2(5);
                }
                else {
                    int remainder = operand % 1000;
                    if (remainder >= instcount) {
                        memoryMap(count, operand-remainder +value);
                        __parseerror2(2);
                    }
                    else {
                        memoryMap(count, operand + value);
                    }
                }
                count++;
            } else if (addressmode == 'E') {
                if(operand>9999){
                    memoryMap(count, 9999);
                    __parseerror2(5);
                }
                else {
                    int remainder = operand % 1000;
                    if (remainder >= usecount) {
                        memoryMap(count, operand);
                        __parseerror2(2);
                    } else {
                        string searchSymbol = uselist[remainder];
                        actuallyUsed.insert(searchSymbol);
                        if (!symbol[searchSymbol]){
                            symbolPass2 = searchSymbol;
                            int valueOperand = (operand - remainder) + 0;
                            memoryMap(count,valueOperand);
                            __parseerror2(6);
                        }
                        else {
                            int valueOperand = (operand - remainder) + symbol[searchSymbol];
                            memoryMap(count, valueOperand);
                        }
                    }
                }
                count++;
            } else if (addressmode == 'A') {
                if(operand>9999){
                    memoryMap(count, 9999);
                    __parseerror2(5);
                }
                else {
                    int remainder = operand % 1000;
                    if (remainder > 511) {
                        memoryMap(count, operand - remainder);
                        __parseerror2(0);
                    } else {
                        memoryMap(count, operand);
                    }
                }
                count++;
            } else {
                if(operand>9999){
                    memoryMap(count, 9999);
                    __parseerror2(4);
                }
                else {
                    memoryMap(count, operand);
                }
                count++;
            }
        }
        value = value + instcount;
        set<string> useListSet;
        for(int i =0; i< usecount; i++){
            useListSet.insert(uselist[i]);
        }
        checkUnusedSymbols(&useListSet, &actuallyUsed);
        module++;
    }
    // Print new line
    addNewLine();
    checkUndefinedSymbols();
    InputFile.close();
}

void Pass1() {
    InputFile.open(file, ios::in);
    int value = 0;
    while (!InputFile.eof()) {

        int defcount = readInt();
        if (defcount == -1){
            continue;
        }
        if (defcount > 16) {
            pass1error = 4;
            __parseerror(pass1error);
            exit(-1);
        }
        for (int i = 0; i < defcount; i++) {
            char *sym = readSym();
            int val = readInt();
            while (!InputFile.eof() && val == -1){
                val = readInt();
            }
            if(val == -1 ){
                __parseerror(0);
                exit(-1);
            }
            value = val + value;
            string key = string(sym);
            if (key.length() >16){
                __parseerror(3);
                exit(-1);
            }
            if (symbol.find(key) != symbol.end()){
               // __parseerror2(3);
                symbolError[key] = " Error: This variable is multiple times defined; first value used";
            }
            else {
                symbol[key] = value;
                symbolModule[key] = module;
                symbolSizeCheck[key] = val;
            }
            //createSymbol(sym, value);
            value = value - val;
        }
        int usecount = readInt();
        if (usecount == -1){
            continue;
        }
        if (usecount > 16) {
            pass1error = 5;
            __parseerror(5);
            exit(-1);
        }
        for (int i = 0; i < usecount; i++) {
            char *sym = readSym();
        }
        int instcount = readInt();

        checkSymbolSize(instcount, value);
        if (instcount == -1){
            continue;
        }
        if (instcount >= 511) {
            __parseerror(6);
            exit(-1);
        }
        value = value + instcount;
        for (int i = 0; i < instcount; i++) {
            char addressmode = readAEIR();
            int operand = readInt();
        }
        offset = 0;
        module++;
    }
    module=1;
    InputFile.close();
    createSymbol();
    Pass2();

}

int main() {
    //cin >> file;
    file = "/Users/rimmyaulakh/CLionProjects/lab1OS/input.txt";
    Pass1();
}

