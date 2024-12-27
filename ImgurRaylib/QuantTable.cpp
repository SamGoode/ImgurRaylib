#include "QuantTable.h"

#include <iostream>

#include "JPEG.h"

QuantTable::QuantTable(const std::string& data, int startIndex) {
    int index = startIndex;

    length = getBytesAsInt(data, index, 2);
    index += 2;

    int byte = getBytesAsInt(data, index, 1);
    destination = byte >> 4;
    tableID = byte & 0x0F;
    index++;

    std::copy(&data[index], &data[index + 64], table);
    index += 64;

    // 
    // order yx
    // 00, 01, 10, 20, 11, 02, 03, 12, 21, 30
}

void QuantTable::print() {
    std::cout << "Header length: " << length << std::endl;
    std::cout << "Destination: " << destination << std::endl;
    std::cout << "ID: " << tableID << std::endl;

    std::cout << "Table data: " << std::endl;
    for (int i = 0; i < 8; i++) {
        for (int n = 0; n < 8; n++) {
            std::cout << (int)table[i + n] << ' ';
        }
        std::cout << std::endl;
    }
}