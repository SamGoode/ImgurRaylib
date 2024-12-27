#pragma once

#include <string>


int getBytesAsInt(const std::string& data, int index, int bytes);

struct Component {
    int ID;
    int samplingY;
    int samplingX;
    int quantID;
};

class JPEG {
private:
    enum Header {
        NA,
        SOI,
        APP0,
        APP2,
        DQT,
        SOF,
        DHT,
        SOS,
        EOI
    };

    const std::string headerNames[9] = {
        "Unknown",
        "Start of Image",
        "Application0",
        "Application2",
        "Define Quantization Table",
        "Start of Frame",
        "Define Huffman Table",
        "Start of Scan",
        "End of Image"
    };

private:
    Header headers[64];
    int headerPositions[64];
    int headerCount = 0;

    int imageHeight;
    int imageWidth;
    int componentCount;
    Component components[3];

public:
    JPEG() {}

    void getHeaders(const std::string& data);
    void printHeaders();

    void getDataSOF(const std::string& data);
    void printImageData();
};