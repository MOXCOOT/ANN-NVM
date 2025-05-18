#pragma once

#include "../config.h"
// #include "logger.h"
#include "../../util/time/time.h"
#include <functional>
#include <memory>
#include <stdlib.h>
#include <vector>

class Parser
{
  private:
    const int ONE_BASED_LIBSVM = 1;
    const int MAX_LINE = 10000000;
    std::function< void( size_t, std::vector< std::pair< size_t, value_t > > ) > consume;

    virtual std::vector< int > tokenize( char* buff )
    {
        std::vector< int > ret;
        int i = 0;
        while ( *( buff + i ) != '\0' )
        {
            if ( *( buff + i ) == ':' || *( buff + i ) == ' ' )
                ret.push_back( i );
            ++i;
        }
        return ret;
    }

    virtual std::vector< std::pair< size_t, value_t > > parse( std::vector< int > tokens, char* buff )
    {
        std::vector< std::pair< size_t, value_t > > ret;
        ret.reserve( tokens.size() / 2 );
        for ( int i = 0; i + 1 < tokens.size(); i += 2 )
        {
            size_t index;
            value_t val;
            sscanf( buff + tokens[i] + 1, "%zu", &index );
            index -= ONE_BASED_LIBSVM;
            double tmp;
            sscanf( buff + tokens[i + 1] + 1, "%lf", &tmp );
            val = tmp;
            ret.push_back( std::make_pair( index, val ) );
        }
        return ret;
    }

  public:
    Parser( const char* path, std::function< void( size_t, std::vector< std::pair< size_t, value_t > > ) > consume ) : consume( consume )
    {
        auto fp = fopen( path, "r" );
        if ( fp == NULL )
        {
            // Logger::log( Logger::ERROR, "File not found at (%s)\n", path );
            exit( 1 );
        }
        std::unique_ptr< char[] > buff( new char[MAX_LINE] );
        std::vector< std::string > buffers;
        size_t idx = 0;
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

class LibSVMParser : public Parser
{
  public:
    using Parser::Parser;
};

class BigANNParser
{
  private:
    std::function< void( size_t, std::vector< std::pair< size_t, value_t > > ) > consume;

    std::vector< std::pair< size_t, value_t > > parse( uint8_t* data, int dim, int vector_index )
    {
        std::vector< std::pair< size_t, value_t > > ret;
        ret.reserve( dim );
        for ( int j = 0; j < dim; ++j )
        {
            ret.push_back( std::make_pair( static_cast< size_t >( j ), static_cast< value_t >( data[vector_index * dim + j] ) ) );
        }
        return ret;
    }

  public:
    BigANNParser( const char* path, std::function< void( size_t, std::vector< std::pair< size_t, value_t > > ) > consume, int dim = 128, size_t num_vectors = 1000000000 ) : consume( consume )
    {
        FILE* fp = fopen( path, "rb" );
        Timer timer;
        if ( fp == NULL )
        {
            // Logger::log( Logger::ERROR, "File not found at (%s)\n", path );
            exit( 1 );
        }

        // // 读取文件头获取向量数量等元数据
        // int num_vectors = 10000;

        std::cout << "num_vectors read from file: " << num_vectors << std::endl;
        // 计算总数据量
        int total_data = num_vectors * dim;
        std::unique_ptr< uint8_t[] > data( new uint8_t[total_data] );
        fread( data.get(), sizeof( uint8_t ), total_data, fp );

        for ( size_t i = 0; i < num_vectors; ++i )
        {
            auto values = parse( data.get(), dim, static_cast< int >( i ) );
            if ( i % 10000000 == 0 )
            {
                std::cout << "Processing vector " << i << "..." << std::endl;
                std::cout << "time consumed: " << timer.elapsed() << std::endl;
            }
            // consume( i, values ); // 为每个向量分配不同的 idx
        }
        std::cout << "time consumed in reading file: " << timer.elapsed() << std::endl;
        fclose( fp );
    }
};