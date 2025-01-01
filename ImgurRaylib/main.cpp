#include "raylib.h"
#include "rlgl.h"

#include <string>
#include <iostream>
#include <fstream>

#include "ImgurRequest.h"
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
    path = "bV8hSec.jpeg";

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

    int imgHeight = jpeg.getImageHeight();
    int imgWidth = jpeg.getImageWidth();

    int* imageData = new int[imgHeight * imgWidth];
    jpeg.decode(data, imageData);

    int screenWidth = 1800;
    int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "ImgurRaylib");

    SetTargetFPS(240);

    Color* pixels = new Color[imgHeight * imgWidth];
    for (int i = 0; i < imgHeight * imgWidth; i++) {
        pixels[i] = convertIntToColor(imageData[i]);
    }

    delete[] imageData;
    
    Image img = {
        .data = pixels,
        .width = imgWidth,
        .height = imgHeight,
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

        DrawTextureEx(texture, {0, 0}, 0.f, 0.5f, WHITE);

        DrawFPS(10, 10);

        EndDrawing();
    }
}
