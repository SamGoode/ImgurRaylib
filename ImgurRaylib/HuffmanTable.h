#pragma once

#include <string>




class HuffmanTable {
private:
    int length;
    int classType;
    int tableID;

    unsigned char bitLengths[16];
    int elementCount;
    unsigned char* elements = nullptr;

public:
    HuffmanTable(const std::string& data, int startIndex);

    ~HuffmanTable() {
        delete[] elements;
    }

    const unsigned char* getBitLengths() const {
        return bitLengths;
    }

    int getElementCount() const {
        return elementCount;
    }

    const unsigned char* getElements() const {
        return elements;
    }

    void print();
};