#pragma once

#include <string>


int getBytesAsInt(const std::string& data, int index, int bytes);

struct QTComponent {
    int ID;
    int samplingY;
    int samplingX;
    int quantID;
};

struct HTComponent {

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
    QTComponent components[3];

    std::string ECS;

public:
    JPEG() {}

    void getHeaders(const std::string& data);
    void printHeaders();

    void getDataSOF(const std::string& data);
    void printImageInfo();

    void parseImageData(const std::string& data, int startIndex);

    void decode(const std::string& data);
};