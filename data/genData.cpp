#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cstdint>

void generateRandomData(int numLines, const std::string& filename) {
    std::ofstream file(filename, std::ios::binary); // 以二进制模式打开文件
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    // 写入每行的字节数（包括前面的 0/1 标志）
    uint8_t bytesPerLine = 129; // 每行 128 个数据字节 + 1 个标志字节
    file.write(reinterpret_cast<char*>(&bytesPerLine), sizeof(bytesPerLine));

    // 生成随机数据并写入文件
    for (int i = 0; i < numLines; ++i) {
        // 随机生成 0 或 1 作为行前标志
        uint8_t lineFlag = static_cast<uint8_t>(rand() % 2);
        file.write(reinterpret_cast<char*>(&lineFlag), sizeof(lineFlag));

        // 随机生成 128 个字节的数据
        uint8_t randomValue;
        for (int j = 0; j < 128; ++j) {
            randomValue = static_cast<uint8_t>(rand() % 256);
            file.write(reinterpret_cast<char*>(&randomValue), sizeof(randomValue));
        }
    }

    file.close();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <number_of_lines>" << std::endl;
        return 1;
    }

    int numLines = std::atoi(argv[1]);
    if (numLines <= 0) {
        std::cerr << "Number of lines must be a positive integer." << std::endl;
        return 1;
    }

    srand(static_cast<unsigned int>(time(0)));
    generateRandomData(numLines, "output.bin"); // 保存为二进制文件
    std::cout << "Generated " << numLines << " lines of data and saved to output.bin" << std::endl;

    return 0;
}