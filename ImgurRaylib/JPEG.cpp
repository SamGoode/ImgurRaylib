#include "JPEG.h"

#include <iostream>

int getBytesAsInt(const std::string& data, int index, int bytes) {
    if (bytes < 0 || bytes > 4) {
        std::cout << "Error: bytes must be between 0-4." << std::endl;
        return -1;
    }

    int result = 0;
    for (int i = 0; i < bytes; i++) {
        result <<= 8;
        result += (int)(unsigned char)data[index + i];
    }

    return result;
}

void JPEG::getHeaders(const std::string& data) {
    headerCount = 0;

    int offset = 0;
    while (offset < data.length()) {
        int marker = getBytesAsInt(data, offset, 2);

        if ((marker & 0xFF00) != 0xFF00) {
            offset++;
            continue;
        }

        if (marker == 0xFFD8) {
            headers[headerCount] = SOI;
            headerPositions[headerCount] = offset;
            headerCount++;

            offset += 2;
            continue;
        }

        if (marker == 0xFFE0) {
            headers[headerCount] = APP0;
            headerPositions[headerCount] = offset;
            headerCount++;

            offset += 2;

            int length = getBytesAsInt(data, offset, 2);
            offset += length;
            continue;
        }

        if (marker == 0xFFE2) {
            headers[headerCount] = APP2;
            headerPositions[headerCount] = offset;
            headerCount++;

            offset += 2;

            int length = getBytesAsInt(data, offset, 2);
            offset += length;
            continue;
        }

        if (marker == 0xFFDB) {
            headers[headerCount] = DQT;
            headerPositions[headerCount] = offset;
            headerCount++;

            offset += 2;

            int length = getBytesAsInt(data, offset, 2);
            offset += length;
            continue;
        }

        if (marker == 0xFFC0) {
            headers[headerCount] = SOF;
            headerPositions[headerCount] = offset;
            headerCount++;

            offset += 2;

            int length = getBytesAsInt(data, offset, 2);
            offset += length;
            continue;
        }

        if (marker == 0xFFC4) {
            headers[headerCount] = DHT;
            headerPositions[headerCount] = offset;
            headerCount++;

            offset += 2;

            int length = getBytesAsInt(data, offset, 2);
            offset += length;
            continue;
        }

        if (marker == 0xFFDA) {
            headers[headerCount] = SOS;
            headerPositions[headerCount] = offset;
            headerCount++;

            offset += 2;

            int length = getBytesAsInt(data, offset, 2);
            offset += length;

            offset = data.length() - 2; // Temporary for now
            continue;
        }

        if (marker == 0xFFD9) {
            headers[headerCount] = EOI;
            headerPositions[headerCount] = offset;
            headerCount++;

            offset += 2;
            continue;
        }

        if (marker == 0xFF00) {
            offset += 2;
            continue;
        }

        headers[headerCount] = NA;
        headerPositions[headerCount] = offset;
        headerCount++;

        std::cout << "Unknown marker located at index " << offset << std::endl;
        std::cout << "Marker is: " << std::hex << marker << std::dec << std::endl;

        offset += 2;
    }
}

void JPEG::printHeaders() {
    for (int i = 0; i < headerCount; i++) {
        Header header = headers[i];
        std::cout << headerNames[header] << " located at " << headerPositions[i] << std::endl;
    }
}

void JPEG::getDataSOF(const std::string& data) {
    int index;
    for (int i = 0; i < headerCount; i++) {
        if (headers[i] == SOF) {
            index = headerPositions[i];
            break;
        }
    }

    int marker = getBytesAsInt(data, index, 2);
    if (marker != 0xFFC0) {
        std::cout << "Error: Incorrect marker." << std::endl;
        return;
    }
    index += 2;

    int length = getBytesAsInt(data, index, 2);
    index += 2;

    int precision = getBytesAsInt(data, index, 1);
    index++;

    imageHeight = getBytesAsInt(data, index, 2);
    imageWidth = getBytesAsInt(data, index + 2, 2);
    index += 4;

    componentCount = getBytesAsInt(data, index, 1);
    for (int i = 0; i < componentCount; i++) {
        int id = getBytesAsInt(data, 764 + (i * 3), 1);
        int sampling = getBytesAsInt(data, 764 + (i * 3) + 1, 1);
        int y = sampling >> 4;
        int x = sampling & 0x0F;
        int quantID = getBytesAsInt(data, 764 + (i * 3) + 2, 1);

        components[i] = { id, y, x, quantID };
    }
}

void JPEG::printImageInfo() {
    std::cout << "Image Height: " << imageHeight << std::endl;
    std::cout << "Image Width: " << imageWidth << std::endl;

    std::cout << "Components: " << componentCount << std::endl;
    for (int i = 0; i < componentCount; i++) {
        std::cout << "ID: " << components[i].ID << '|';
        std::cout << "Sampling: " << components[i].samplingY << 'x' << components[i].samplingX << '|';
        std::cout << "Quant ID: " << components[i].quantID << '|';
        std::cout << std::endl;
    }
}

void JPEG::parseImageData(const std::string& data, int startIndex) {
    ECS = "";

    int index = startIndex;
    int latestIndex = startIndex;

    int FF00bytes = 0;
    while (index < data.length()) {
        int bytePair = getBytesAsInt(data, index, 2);

        if (bytePair == 0xFF00) {
            std::cout << std::hex << bytePair << std::dec;
            std::cout << " byte found at byte index: " << index - startIndex << std::endl;

            FF00bytes++;
            ECS.append(&data[latestIndex], &data[index + 1]);

            index += 2;
            latestIndex = index;
            continue;
        }

        if (bytePair == 0xFFD9) {
            std::cout << std::hex << bytePair << std::dec;
            std::cout << " byte found at byte index: " << index - startIndex << std::endl;

            ECS.append(&data[latestIndex], &data[index]);
            break;
        }

        index++;
    }
    
    std::cout << std::hex << (int)(unsigned char)ECS[ECS.length() - 1] << std::dec << std::endl;

    std::cout << "FF00 bytes found: " << FF00bytes << std::endl;
}

void JPEG::decode(const std::string& data) {
    int index;
    for (int i = 0; i < headerCount; i++) {
        if (headers[i] == SOS) {
            index = headerPositions[i];
            break;
        }
    }

    int marker = getBytesAsInt(data, index, 2);
    if (marker != 0xFFDA) {
        std::cout << "Error: Incorrect marker." << std::endl;
        return;
    }
    index += 2;

    int length = getBytesAsInt(data, index, 2);
    index += length;
    int startIndex = index;

    parseImageData(data, index);

    //std::cout << "FF00 bytes found: " << FF00bytes << std::endl;
    std::cout << data.length() - startIndex << std::endl;
    std::cout << ECS.length() << std::endl;
    

    for (int i = 0; i < ECS.length(); i++) {
        int bytePair = getBytesAsInt(ECS, i, 2);

        if ((bytePair & 0xFF00) == 0xFF00) {
            std::cout << std::hex << bytePair << std::dec;
            std::cout << " byte found at byte index: " << i << std::endl;
        }
    }
}


