#pragma once

#include "../config.h"
#include <algorithm>
#include <iostream>

class Data
{
  private:
    pobj::persistent_ptr< value_t[] > data;
    pobj::p< size_t > num;
    pobj::p< size_t > curr_num = 0;
    pobj::p< size_t > have_num = 0;
    pobj::p< int > dim;

  public:
    Data( pobj::pool_base& pop, size_t n, int d ) : num( n ), dim( d ), curr_num( 0 )
    {
        pobj::transaction::run( pop, [&] { data = pobj::make_persistent< value_t[] >( n * d ); } );
    }

    int get_dim() const { return dim; }

    // 修改第idx个点的数据（假设每个点有dim维，输入为指向T的指针）
    void set_point( pobj::pool_base& pop, size_t idx, const value_t* value )
    {
        pobj::transaction::run( pop, [&] {
            for ( int i = 0; i < dim; ++i )
            {
                data[idx * dim + i] = value[i];
            }

            curr_num = curr_num + 1; // 更新当前点数
        } );
        std::cout << "curr_num: " << curr_num << std::endl;
    }

    // 添加稀疏向量
    void add( pobj::pool_base& pop, size_t idx, std::vector< std::pair< size_t, value_t > >& value )
    {
        pobj::transaction::run( pop, [&] {
            curr_num = std::max( static_cast< size_t >( curr_num ), idx );
            auto p = get( idx );
            for ( const auto& v : value )
                *( p + v.first ) = v.second;
        } );
    }

    void print()
    {
        std::cout << "Data points (curr_num = " << curr_num << ", dim = " << dim << "):" << std::endl;
        int np = static_cast< size_t >( curr_num ) > 10 ? 10 : static_cast< size_t >( curr_num );
        for ( size_t i = 0; i < np; ++i )
        {
            std::cout << "Point " << i << ": ";
            value_t* p = get( i );
            for ( int j = 0; j < dim; ++j )
            {
                std::cout << p[j];
                if ( j != dim - 1 )
                    std::cout << ", ";
            }
            std::cout << std::endl;
        }
    }

    value_t* get( size_t idx ) const { return data.get() + idx * dim; }

    size_t max_vertices() const
    {
        return num; // 返回最大点数
    }

    // 返回当前点数
    size_t curr_vertices() const { return curr_num; }

    void set_curr_vertices( size_t num )
    {
        pobj::transaction::run( pop, [&] { curr_num = num; } );
    }

    std::vector< std::pair< size_t, value_t > > get_point( size_t idx ) const
    {
        if ( idx >= num )
        {
            throw std::out_of_range( "Index out of range" );
        }

        std::vector< std::pair< size_t, value_t > > ret;
        ret.reserve( dim );
        value_t* p = get( idx );
        for ( int i = 0; i < dim; ++i )
        {
            ret.emplace_back( i, p[i] );
        }
        return ret;
    }

    std::vector< value_t > organize_point( const std::vector< std::pair< size_t, value_t > >& v )
    {
        std::vector< value_t > ret( dim, 0 );
        for ( const auto& p : v )
        {
            if ( p.first >= dim )
                printf( "error %ld %d\n", p.first, static_cast< int >( dim ) );
            ret[p.first] = p.second;
        }
        return std::move( ret );
    }

    void initcurr_num()
    {
        pobj::transaction::run( pop, [&] { curr_num = 0; } );
    }
};
