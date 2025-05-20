#pragma once

#include "../config.h"
#include <cstdio>
#include <functional>
#include <iostream>
#include <memory>
#include <stdlib.h>
#include <vector>

template < typename OutputType >
class Parser
{
  protected:
    const int ONE_BASED_LIBSVM = 1;
    const int MAX_LINE = 10000000;
    std::function< void( size_t, OutputType ) > consume;
    virtual std::vector< int > tokenize( char* buff ) = 0;
    virtual OutputType parse( const std::vector< int >& tokens, char* buff ) = 0;
};

class LibSVMParser : public Parser< std::vector< std::pair< size_t, value_t > > >
{
  private:
    std::vector< std::pair< size_t, value_t > > parse( const std::vector< int >& tokens, char* buff ) override
    {
        std::vector< std::pair< size_t, value_t > > ret;
        ret.reserve( tokens.size() / 2 );
        for ( size_t i = 0; i + 1 < tokens.size(); i += 2 )
        {
            size_t index;
            value_t val;
            sscanf( buff + tokens[i] + 1, "%zu", &index );
            index -= ONE_BASED_LIBSVM;
            double tmp;
            sscanf( buff + tokens[i + 1] + 1, "%lf", &tmp );
            val = static_cast< value_t >( tmp );
            ret.emplace_back( index, val );
        }
        return ret;
    }
    std::vector< int > tokenize( char* buff ) override
    {
        std::vector< int > ret;
        int i = 0;
        while ( buff[i] != '\0' )
        {
            if ( buff[i] == ' ' || buff[i] == ':' )
                ret.push_back( i );
            ++i;
        }
        return ret;
    }

  public:
    LibSVMParser( const char* path, std::function< void( size_t, std::vector< std::pair< size_t, value_t > > ) > consume )
    {
        this->consume = consume;
        auto fp = fopen( path, "r" );
        if ( fp == NULL )
        {
            // 文件不存在，记录错误并退出
            std::cout << "File not found: " << path << std::endl;
            exit( 1 );
        }
        // 分配一行的缓冲区
        std::unique_ptr< char[] > buff( new char[MAX_LINE] );
        std::vector< std::string > buffers; // 未使用，可移除
        size_t idx = 0;                     // 行号计数器
        // 逐行读取文件
        while ( fgets( buff.get(), MAX_LINE, fp ) )
        {
            auto tokens = tokenize( buff.get() );
            auto values = parse( tokens, buff.get() );
            consume( idx, values );
            ++idx;
        }
        fclose( fp );
    }
};

class BigANNParse : public Parser<std::vector<std::pair<size_t, value_t>>>
{
private:
    int dim;
    const char* path;  // 添加一个成员变量来存储路径

    // Override the tokenize method to do nothing since we don't need tokenization for binary data
    std::vector<int> tokenize(char* buff) override {
        return {};
    }

    // Override the parse method to read binary data
    std::vector<std::pair<size_t, value_t>> parse(const std::vector<int>& tokens, char* buff) override {
        std::vector<std::pair<size_t, value_t>> ret;
        ret.reserve(dim);
        uint8_t* data = reinterpret_cast<uint8_t*>(buff);
        for (int j = 0; j < dim; ++j) {
            ret.push_back(std::make_pair(static_cast<size_t>(j), static_cast<value_t>(data[j])));
        }
        return ret;
    }

public:
    BigANNParse(const char* path, 
                std::function<void(size_t, std::vector<std::pair<size_t, value_t>>)> consume, 
                int dim = 128)
        : Parser<std::vector<std::pair<size_t, value_t>>>(),  // 调用父类的默认构造函数
          dim(dim), path(path) {
        // 手动设置父类的成员变量
        this->consume = consume;

        // 打开文件
        FILE* fp = fopen(path, "rb");
        if (fp == NULL) {
            Logger::log(Logger::ERROR, "File not found at (%s)\n", path);
            exit(1);
        }

        // 读取文件头以获取元数据，例如向量数量
        int num_vectors;
        fread(&num_vectors, sizeof(int), 1, fp);

        // 计算总数据大小
        int total_data = num_vectors * dim;
        std::unique_ptr<uint8_t[]> data(new uint8_t[total_data]);
        fread(data.get(), sizeof(uint8_t), total_data, fp);

        // 解析数据并调用回调函数
        for (size_t i = 0; i < num_vectors; ++i) {
            auto values = parse({}, reinterpret_cast<char*>(data.get() + i * dim));
            consume(i, values); // 为每个向量分配不同的索引
        }

        fclose(fp);
    }
};



class mydataParse :  public Parser<std::vector<std::pair<size_t, value_t>>> {
private:
    int dim;
    const char* path;

    // Override the tokenize method to do nothing since we don't need tokenization for binary data
    std::vector<int> tokenize(char* buff) override {
        return {};
    }

    // Override the parse method to read binary data
    std::vector<std::pair<size_t, value_t>> parse(const std::vector<int>& tokens, char* buff) override {
        std::vector<std::pair<size_t, value_t>> ret;
        ret.reserve(dim+1);
        uint8_t* data = reinterpret_cast<uint8_t*>(buff);
        for (int j = 0; j < dim+1; ++j) {
            ret.push_back(std::make_pair(j, static_cast<value_t>(data[j])));
        }
        return ret;
    }

public:
    mydataParse(const char* path, std::function<void(idx_t, std::vector<std::pair<size_t, value_t>>)> consume, int dim = 128)
        : Parser<std::vector<std::pair<size_t, value_t>>>(),  // 调用父类的默认构造函数
          dim(dim), path(path) {
        FILE* fp = fopen(path, "rb");
        if (fp == NULL) {
            Logger::log(Logger::ERROR, "File not found at (%s)\n", path);
            exit(1);
        }

        // Read file header to get metadata like number of vectors and dimensions
        uint8_t bytesPerLine;
        fread(&bytesPerLine, sizeof(uint8_t), 1, fp);

        if (bytesPerLine != dim + 1) { // 检查每行的字节数是否正确
            Logger::log(Logger::ERROR, "Bytes per line mismatch. Expected %d, found %d\n", dim + 1, bytesPerLine);
            fclose(fp);
            exit(1);
        }

        // Buffer to hold one vector at a time (including the flag)
        std::unique_ptr<uint8_t[]> data(new uint8_t[dim + 1]);

        // Read each vector
        int idx = 0;
        while (fread(data.get(), sizeof(uint8_t), dim + 1, fp) == dim + 1) {
            auto values = parse({}, reinterpret_cast<char*>(data.get()));
            consume(idx++, values);
        }

        fclose(fp);
    }
};



