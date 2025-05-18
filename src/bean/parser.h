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