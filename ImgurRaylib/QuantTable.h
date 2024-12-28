#pragma once

#include <string>

class QuantTable {
private:
    int length;
    int destination;
    int tableID;

public:
    unsigned char table[64];

public:
    QuantTable(const std::string& data, int startIndex);

    void print();    
};