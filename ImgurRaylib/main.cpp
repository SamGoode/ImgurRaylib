//#include "raylib.h"
//#include "rlgl.h"

#include <string>
#include <windows.h>

#include <iostream>
#include <fstream>

#include "ImgurRequest.h"
#include "HuffmanTable.h"


class BinaryTree {
private:
    struct Node {
        Node* parent = nullptr;

        virtual ~Node() {}

        virtual bool isFull() = 0;
    };

    struct BranchNode : public Node {
        Node* firstChild = nullptr;
        Node* secondChild = nullptr;

        BranchNode() {}
        BranchNode(const Node& base) {}

        virtual bool isFull() override {
            if (!firstChild || !secondChild) {
                return false;
            }

            return firstChild->isFull() && secondChild->isFull();
        }
    };

    struct LeafNode : public Node {
        unsigned char value;

        virtual bool isFull() override {
            return true;
        }
    };

private:
    Node* nodes[512];
    Node* rootNode = nullptr;
    int nodeCount;

public:
    BinaryTree(const HuffmanTable& ht) {
        const unsigned char* bitLengths = ht.getBitLengths();
        const int elementCount = ht.getElementCount();
        const unsigned char* elements = ht.getElements();

        rootNode = new BranchNode();
        nodes[0] = rootNode;
        nodeCount = 1;

        for (int elementIndex = 0; elementIndex < elementCount; elementIndex++) {
            unsigned char element = elements[elementIndex];
            int bitLength = 0;

            int sum = 0;
            for (int i = 0; i < 16; i++) {
                sum += (int)bitLengths[i];

                if (elementIndex < sum) {
                    bitLength = i + 1;
                    break;
                }
            }

            BranchNode* currentNode = dynamic_cast<BranchNode*>(rootNode);
            for (int i = 0; i < bitLength - 1; i++) {

                if (!currentNode->firstChild) {
                    BranchNode* newNode = addNewBranch();

                    newNode->parent = currentNode;
                    currentNode->firstChild = newNode;

                    currentNode = newNode;
                    continue;
                }

                if (!currentNode->firstChild->isFull()) {
                    currentNode = dynamic_cast<BranchNode*>(currentNode->firstChild);
                    continue;
                }

                if (!currentNode->secondChild) {
                    BranchNode* newNode = new BranchNode();
                    nodes[nodeCount] = newNode;
                    nodeCount++;

                    newNode->parent = currentNode;
                    currentNode->secondChild = newNode;

                    currentNode = newNode;
                    continue;
                }

                if (!currentNode->secondChild->isFull()) {
                    currentNode = dynamic_cast<BranchNode*>(currentNode->secondChild);
                    continue;
                }
            }

            if (currentNode->firstChild == nullptr) {
                LeafNode* newNode = new LeafNode();
                nodes[nodeCount] = newNode;
                nodeCount++;

                newNode->parent = currentNode;
                currentNode->firstChild = newNode;

                newNode->value = element;
            }
            else {
                LeafNode* newNode = new LeafNode();
                nodes[nodeCount] = newNode;
                nodeCount++;

                newNode->parent = currentNode;
                currentNode->secondChild = newNode;

                newNode->value = element;
            }
        }
    }

    ~BinaryTree() {
        for (int i = 0; i < nodeCount; i++) {
            delete nodes[i];
        }
    }

    BranchNode* addNewBranch() {
        BranchNode* newNode = new BranchNode();
        nodes[nodeCount] = newNode;
        nodeCount++;

        return newNode;
    }

    unsigned char getElement(std::string bits) {
        Node* currentNode = rootNode;
        for (int i = 0; i < bits.length(); i++) {
            BranchNode* currentBranch = dynamic_cast<BranchNode*>(currentNode);
            if (!currentBranch) {
                std::cout << "Error: Invalid bitstream." << std::endl;
                return 0;
            }

            if (bits[i] == '0' && currentBranch->firstChild) {
                currentNode = currentBranch->firstChild;
                continue;
            }
            if (bits[i] == '1' && currentBranch->secondChild) {
                currentNode = currentBranch->secondChild;
                continue;
            }
            
            std::cout << "Error: Invalid bitstream." << std::endl;
            return 0;
        }

        LeafNode* elementNode = dynamic_cast<LeafNode*>(currentNode);
        if (elementNode) {
            return elementNode->value;
        }

        std::cout << "Error: Invalid bitstream." << std::endl;
        return 0;
    }
};

int main() {
    std::string path = "nUTJtzV.jpeg";

    ImgurRequest request(path);

    std::string data;
    request.GetImageData(data);

    //std::fstream fileStream;
    //fileStream.open("test.jpeg", std::fstream::out | std::fstream::trunc | std::fstream::binary);
    //fileStream << data;
    //fileStream.close();

    int offset = 0;
    while (offset < data.length()) {
        int marker = ((int)(unsigned char)data[offset] << 8) + (int)(unsigned char)data[offset + 1];
        std::cout << std::hex << marker << std::dec << std::endl;

        if (marker == 0xFFC4) {
            offset += 2;
            break;
        }
        
        offset++;
    }

    std::cout << "Huffman table located at index: " + std::to_string(offset) << std::endl;

    HuffmanTable ht = HuffmanTable(data, offset);
    ht.print();

    BinaryTree bt = BinaryTree(ht);

    int element = (int)bt.getElement("1110");
    std::cout << element << std::endl;

    return 0;

    //int byteCount = response.length();
    //std::cout << "Bytes: " << byteCount << std::endl;

    //enum marker {
    //    NA,
    //    SOI,
    //    APP0,
    //    DQT,
    //    SOF,
    //    DHT,
    //    SOS,
    //    EOI
    //};

    //int offset = 0;
    //int index = 0;
    //marker currentMarker = NA;
    //while (offset + index < byteCount) {
    //    if (response[offset + index] == (char)0xFF) {
    //        bool validMarker = true;
    //        char nextByte = response[offset + index + 1];

    //        if (nextByte == (char)0xD8) {
    //            currentMarker = SOI;
    //            std::cout << "START OF IMAGE" << std::endl;
    //        }
    //        else if (nextByte == (char)0xE0) {
    //            currentMarker = APP0;
    //            std::cout << "APPLICATION0" << std::endl;
    //        }
    //        else if (nextByte == (char)0xDB) {
    //            currentMarker = DQT;
    //            std::cout << "DEFINE QUANTIZATION TABLE" << std::endl;
    //        }
    //        else if (nextByte == (char)0xC0) {
    //            currentMarker = SOF;
    //            std::cout << "START OF FRAME" << std::endl;
    //        }
    //        else if (nextByte == (char)0xC4) {
    //            currentMarker = DHT;
    //            std::cout << "DEFINE HUFFMAN TABLE" << std::endl;
    //        }
    //        else if (nextByte == (char)0xDA) {
    //            currentMarker = SOS;
    //            std::cout << "START OF SCAN" << std::endl;
    //        }
    //        else if (nextByte == (char)0xD9) {
    //            currentMarker = EOI;
    //            std::cout << "END OF IMAGE" << std::endl;
    //        }
    //        else {
    //            validMarker = false;
    //        }

    //        if (validMarker) {
    //            offset += index + 2;
    //            index = 0;

    //            continue;
    //        }
    //    }

    //    //char* bytes;
    //    //int length;

    //    switch (currentMarker) {
    //    case DQT:
    //    {
    //        char* bytes = &(response[offset + index]);

    //        int length = ((int)bytes[0] << 8) + (int)bytes[1];
    //        int destination = (int)bytes[2];

    //        std::cout << "length: " << length << std::endl;
    //        std::cout << "destination: " << destination << std::endl;

    //        index += length;
    //    }
    //        break;
    //    case SOF:
    //    {
    //        char* bytes = &(response[offset + index]);

    //        int length = ((int)bytes[0] << 8) + (int)bytes[1];
    //        int precision = (int)bytes[2];
    //        int lineNb = (int)bytes[4];
    //        int lines = (int)bytes[6];
    //        int components = (int)bytes[7];

    //        std::cout << "length: " << length << std::endl;
    //        std::cout << "precision: " << precision << std::endl;
    //        std::cout << "lineNb: " << lineNb << std::endl;
    //        std::cout << "lines: " << lines << std::endl;
    //        std::cout << "components: " << components << std::endl;

    //        index += length;
    //    }
    //        break;
    //    }

    //    //int byte = 0x000000FF & (int)response[offset + index];
    //    //std::cout << std::hex << byte << std::dec << std::endl;

    //    //index++;
    //}


    //for (int i = 0; i < 2000; i++) {
    //    if (response[i] == (char)0xFF) {
    //        if (response[i + 1] == (char)0xD8) {
    //            std::cout << "start of image" << std::endl;
    //            
    //            i++;
    //            continue;
    //        }
    //        else if (response[i + 1] == (char)0xE0) {
    //            std::cout << "Application0" << std::endl;

    //            i++;
    //            continue;
    //        }
    //        else if (response[i + 1] == (char)0xDB) {
    //            std::cout << "define quantization table" << std::endl;

    //            i++;
    //            continue;
    //        }
    //        else if (response[i + 1] == (char)0xC0) {
    //            std::cout << "start of frame" << std::endl;

    //            i++;
    //            continue;
    //        }
    //        else if (response[i + 1] == (char)0xC4) {
    //            std::cout << "define huffman table" << std::endl;

    //            i++;
    //            continue;
    //        }
    //        else if (response[i + 1] == (char)0xDA) {
    //            std::cout << "start of scan" << std::endl;

    //            i++;
    //            continue;
    //        }
    //        else if (response[i + 1] == (char)0xD9) {
    //            std::cout << "end of image" << std::endl;

    //            break;
    //        }
    //    }
    //    
    //    
    //    int byte = 0x000000FF & (int)response[i];
    //    std::cout << std::hex << byte << std::endl;
    //}
    //std::cout << std::dec;


    //std::fstream fileStream;
    //fileStream.open("test.jpeg", std::fstream::out | std::fstream::trunc | std::fstream::binary);
    //fileStream << response;
    //fileStream.close();


    //int screenWidth = 1600;
    //int screenHeight = 800;

    //InitWindow(screenWidth, screenHeight, "ImgurRaylib");

    ////SetTargetFPS(240);


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
