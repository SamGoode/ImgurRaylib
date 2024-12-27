//#include "raylib.h"
//#include "rlgl.h"

#include <string>
#include <windows.h>

#include <iostream>
#include <fstream>

#include "ImgurRequest.h"
#include "QuantTable.h"
#include "HuffmanTable.h"
#include "HuffmanTree.h"

#include "JPEG.h"



int main() {
    std::string path = "nUTJtzV.jpeg";

    ImgurRequest request(path);

    std::string data;
    request.GetImageData(data);

    //std::fstream fileStream;
    //fileStream.open("test.jpeg", std::fstream::out | std::fstream::trunc | std::fstream::binary);
    //fileStream << data;
    //fileStream.close();

    JPEG jpeg;
    jpeg.getHeaders(data);
    jpeg.printHeaders();

    std::cout << std::endl;

    jpeg.getDataSOF(data);
    jpeg.printImageInfo();

    jpeg.decode(data);

    // QT at 616
    //std::cout << "Quantization table located at byte index: 616" << std::endl;
    //QuantTable qTable = QuantTable(data, 618);
    //qTable.print();

    // HT at 773
    //std::cout << "Huffman table located at byte index: 773" << std::endl;
    //HuffmanTable hTable = HuffmanTable(data, 775);
    //hTable.print();

    //HuffmanTree hTree = HuffmanTree(hTable);
    //int element = (int)hTree.getElement("10");
    //std::cout << element << std::endl;

    return 0;


    //int screenWidth = 1600;
    //int screenHeight = 800;

    //InitWindow(screenWidth, screenHeight, "ImgurRaylib");

    //SetTargetFPS(240);


    //while (!WindowShouldClose()) {
    //    // Updates
    //    float delta = GetFrameTime();
    //    
    //    
    //    // Drawing
    //    BeginDrawing();

    //    ClearBackground(RAYWHITE);


    //    DrawFPS(10, 10);

    //    EndDrawing();
    //}
}
