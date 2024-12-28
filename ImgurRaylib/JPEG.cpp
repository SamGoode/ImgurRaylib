#include "JPEG.h"

#include <iostream>
#include <algorithm>

#include "QuantTable.h"
#include "HuffmanTable.h"
#include "HuffmanTree.h"
#include "BitStream.h"

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
        int quantID = getBytesAsInt(data, 764 + (i * 3) + 2, 1);

        components[i] = { id, sampling, quantID };
    }
}

void JPEG::printImageInfo() {
    std::cout << "Image Height: " << imageHeight << std::endl;
    std::cout << "Image Width: " << imageWidth << std::endl;

    std::cout << "Components: " << componentCount << std::endl;
    for (int i = 0; i < componentCount; i++) {
        std::cout << "ID: " << components[i].ID << '|';

        int s = components[i].sampFactor;
        
        std::cout << "Sampling Factor: " << (s >> 4) << (s & 0x0F) << '|';
        std::cout << "Quant ID: " << components[i].quantID << '|';
        std::cout << std::endl;
    }
}

void JPEG::parseImageData(const std::string& data, int startIndex) {
    ECS = "";

    int index = startIndex;
    int latestIndex = startIndex;

    while (index < data.length()) {
        int bytePair = getBytesAsInt(data, index, 2);

        if (bytePair == 0xFF00) {
            ECS.append(&data[latestIndex], &data[index + 1]);

            index += 2;
            latestIndex = index;
            continue;
        }

        if (bytePair == 0xFFD9) {
            ECS.append(&data[latestIndex], &data[index]);
            break;
        }

        index++;
    }
}

int JPEG::decodeRLC(int size, int bits) {
    bool firstBit = bits >> (size - 1);
    
    if (firstBit) { // Positive;
        return bits;
    }
    else { // Negative
        return bits - ((1 << size) - 1);
    }
}



void undoZigzag(const int input[64], int output[64]) {
    bool state = 0;
    int y = 0;
    int x = 0;

    int index = 0;
    while (index < 64) {
        output[index] = input[y * 8 + x];

        switch (state) {
        case 0:
            y--;
            x++;

            if (x > 7) {
                y += 2;
                x--;
                state = 1;
            }
            else if (y < 0) {
                y++;
                state = 1;
            }
            break;
        case 1:
            y++;
            x--;

            if (y > 7) {
                y--;
                x += 2;
                state = 0;
            }
            else if (x < 0) {
                x++;
                state = 0;
            }
            break;
        }

        index++;
    }
}

static float normFactor(int n) {
    if (n == 0) {
        return sqrt(2) / 2;
    }
    else {
        return 1;
    }
}


void JPEG::inverseDCT(int DCT[64], int output[64]) {
    int matrixDCT[64];
    undoZigzag(DCT, matrixDCT);

    //std::cout << std::endl;
    //for (int y = 0; y < 8; y++) {
    //    for (int x = 0; x < 8; x++) {
    //        std::cout << matrixDCT[y * 8 + x] << ' ';
    //    }
    //    std::cout << std::endl;
    //}
    
    float pi = acos(-1);

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            float sum = 0.f;

            for (int u = 0; u < 8; u++) {
                for (int v = 0; v < 8; v++) {
                    float normFactors = normFactor(u) * normFactor(v);
                    float cosArgs = cos(((2 * x + 1) * u * pi) / 16) * cos(((2 * y + 1) * v * pi) / 16);
                    sum += normFactors * (float)matrixDCT[v * 8 + u] * cosArgs;
                }
            }

            output[y * 8 + x] = round(sum / 4);
        }
    }

    //std::cout << std::endl;
    //for (int y = 0; y < 8; y++) {
    //    for (int x = 0; x < 8; x++) {
    //        std::cout << output[y * 8 + x] << ' ';
    //    }
    //    std::cout << std::endl;
    //}
}

void JPEG::buildMCU(BitStream& stream, QuantTable& qTable, HuffmanTree& hTreeDC, HuffmanTree& hTreeAC, int& lastDcCoeff, int output[64]) {
    int DCT[64] = {0};
    
    int size = hTreeDC.getCodeFromStream(stream);
    int bits = 0;
    if (size > 0) {
        bits = stream.getBits(size);
    }

    int dcCoeff = decodeRLC(size, bits) + lastDcCoeff;
    lastDcCoeff = dcCoeff;

    DCT[0] = dcCoeff * qTable.table[0];

    for (int i = 1; i < 64; i++) {
        unsigned char byte = hTreeAC.getCodeFromStream(stream);
        if (byte == 0x00) {
            break;
        }

        int runLength = byte >> 4;
        i += runLength;

        int size = byte & 0x0F;
        int bits = stream.getBits(size);

        int acCoeff = decodeRLC(size, bits);
        DCT[i] = acCoeff * qTable.table[i];
    }


    inverseDCT(DCT, output);
}

int convertToRGBA(int Y, int Cb, int Cr) {
    float Bf = Cb * (2 - 2 * 0.114f) + Y;
    float Rf = Cr * (2 - 2 * 0.299f) + Y;
    float Gf = (Y - 0.114f * Bf - 0.299f * Rf) / 0.587f;


    int R = std::clamp((int)round(Rf + 128), 0, 255);
    int G = std::clamp((int)round(Gf + 128), 0, 255);
    int B = std::clamp((int)round(Bf + 128), 0, 255);

    //std::cout << R << ' ' << G << ' ' << B << std::endl;

    int rgba = (R << 24) + (G << 16) + (B << 8) + 0xFF;

    return rgba;
}

void JPEG::decode(const std::string& data, int* output) {
    int index = 0;
    for (int i = 0; i < headerCount; i++) {
        if (headers[i] == SOS) {
            index = headerPositions[i];
            break;
        }
    }

    //Define Quantization Table located at 616
    //Define Quantization Table located at 685
    //Start of Frame located at 754
    //Define Huffman Table located at 773
    //Define Huffman Table located at 803
    //Define Huffman Table located at 868
    //Define Huffman Table located at 895

    QuantTable qTableLum = QuantTable(data, 618);
    QuantTable qTableChrome = QuantTable(data, 687);

    HuffmanTable hTableLumDC = HuffmanTable(data, 775);
    HuffmanTable hTableLumAC = HuffmanTable(data, 805);
    HuffmanTable hTableChromeDC = HuffmanTable(data, 870);
    HuffmanTable hTableChromeAC = HuffmanTable(data, 897);

    HuffmanTree hTreeLumDC = HuffmanTree(hTableLumDC);
    HuffmanTree hTreeLumAC = HuffmanTree(hTableLumAC);
    HuffmanTree hTreeChromeDC = HuffmanTree(hTableChromeDC);
    HuffmanTree hTreeChromeAC = HuffmanTree(hTableChromeAC);

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

    int lastLumDcCoeff0 = 0;
    int lastLumDcCoeff1 = 0;
    int lastLumDcCoeff2 = 0;
    int lastLumDcCoeff3 = 0;

    int lastCbdDcCoeff = 0;
    int lastCrdDcCoeff = 0;

    BitStream stream = BitStream(ECS.c_str());

    int lumMat[4][64];

    int cbdMat[64];
    int crdMat[64];

    int blockCount = imageHeight * imageWidth / 256;
    int blocksRead = 0;

    std::cout << std::endl;

    for (int blockY = 0; blockY < imageHeight / 16; blockY++) {
        for (int blockX = 0; blockX < imageWidth / 16; blockX++) {
            buildMCU(stream, qTableLum, hTreeLumDC, hTreeLumAC, lastLumDcCoeff0, lumMat[0]);
            buildMCU(stream, qTableLum, hTreeLumDC, hTreeLumAC, lastLumDcCoeff0, lumMat[1]);
            buildMCU(stream, qTableLum, hTreeLumDC, hTreeLumAC, lastLumDcCoeff0, lumMat[2]);
            buildMCU(stream, qTableLum, hTreeLumDC, hTreeLumAC, lastLumDcCoeff0, lumMat[3]);

            buildMCU(stream, qTableChrome, hTreeChromeDC, hTreeChromeAC, lastCbdDcCoeff, cbdMat);
            buildMCU(stream, qTableChrome, hTreeChromeDC, hTreeChromeAC, lastCrdDcCoeff, crdMat);


            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    int imageX = blockX * 16 + x;
                    int imageY = blockY * 16 + y;

                    int lumMatIndex = (y / 8) * 2 + (x / 8);


                    output[imageY * imageWidth + imageX] = convertToRGBA(lumMat[lumMatIndex][(y % 8) * 8 + (x % 8)], cbdMat[(y / 2) * 8 + (x / 2)], crdMat[(y / 2) * 8 + (x / 2)]);
                }
            }


            //std::cout << lumMat[0][0] << ' ' << cbdMat[0] << ' ' << crdMat[0] << std::endl;
            //for (int i = 0; i < 4; i++) {
            //    int rgba = output[0];

            //    std::cout << ((rgba >> 24 - (8 * i)) & 0xFF) << ' ';
            //}
            //std::cout << std::endl;

            //return;

            blocksRead++;
            std::cout << '\r' << blocksRead << " / " << blockCount << " MCU blocks read" << std::flush;
        }
    }


    //for (int blockY = 0; blockY < imageHeight / 8; blockY++) {
    //    for (int blockX = 0; blockX < imageWidth / 8; blockX++) {
    //        buildMCU(stream, qTableLum, hTreeLumDC, hTreeLumAC, lastLumDcCoeff, lumMat);
    //        buildMCU(stream, qTableChrome, hTreeChromeDC, hTreeChromeAC, lastCrdDcCoeff, crdMat);
    //        buildMCU(stream, qTableChrome, hTreeChromeDC, hTreeChromeAC, lastCbdDcCoeff, cbdMat);


    //        for (int y = 0; y < 8; y++) {
    //            for (int x = 0; x < 8; x++) {
    //                int imageX = blockX * 8 + x;
    //                int imageY = blockY * 8 + y;

    //                output[imageY * imageWidth + imageX] = convertToRGBA(lumMat[y * 8 + x], cbdMat[y * 8 + x], crdMat[y * 8 + x]);
    //            }
    //        }
    //    }
    //}
}

