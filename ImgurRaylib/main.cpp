#include "raylib.h"
#include "rlgl.h"

#include <string>
#include <iostream>
#include <fstream>

#include "ImgurRequest.h"
#include "QuantTable.h"
#include "HuffmanTable.h"
#include "HuffmanTree.h"

#include "JPEG.h"

Color convertIntToColor(int rgba) {
    unsigned char r = rgba >> 24;
    unsigned char g = rgba >> 16;
    unsigned char b = rgba >> 8;
    unsigned char a = rgba;

    return { r, g, b, a };
}

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

    int* imageData = new int[4032 * 3024];
    jpeg.decode(data, imageData);

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




    //Color test = convertIntToColor(imageData[0]);
    //int col = imageData[0];

    //std::cout << ((col >> 0) & 0xFF) << std::endl;
    //std::cout << (int)test.a << std::endl;

    //return 0;

    int screenWidth = 1800;
    int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "ImgurRaylib");

    SetTargetFPS(240);

    Color* pixels = new Color[4032 * 3024];
    for (int i = 0; i < 4032 * 3024; i++) {
        pixels[i] = convertIntToColor(imageData[i]);
    }

    delete[] imageData;
    
    Image img = {
        .data = pixels,
        .width = 3024,
        .height = 4032,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    Texture2D texture = LoadTextureFromImage(img);
    delete[] pixels;
    
    while (!WindowShouldClose()) {
        // Updates
        float delta = GetFrameTime();
        
        
        // Drawing
        BeginDrawing();

        ClearBackground(RAYWHITE);

        //DrawTexture(texture, 0, 0, WHITE);

        DrawTextureEx(texture, {0, 0}, 0.f, 0.25f, WHITE);

        DrawFPS(10, 10);

        EndDrawing();
    }
}
