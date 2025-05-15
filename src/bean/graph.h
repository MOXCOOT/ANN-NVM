#pragma once

#include "data.h"

class Graph
{
  public:
    // virtual void add_vertex( size_t vertex_id, std::vector< std::pair< int, value_t > >& point ) = 0;
    // virtual void search_top_k( const std::vector< std::pair< int, value_t > >& query, int k, std::vector< size_t >& result ) = 0;
    virtual void search_top_k_batch( const std::vector< std::vector< std::pair< int, value_t > > >& queries, int k, std::vector< std::vector< size_t > >& results ){};
    virtual ~Graph(){};
};

template < DistType dist_type >
class FixedDegreeGraph : public Graph
{
  public:
    pobj::p< const int > degree = SEARCH_DEGREE;
    pobj::p< const int > flexible_degree = FIXED_DEGREE;
    pobj::p< const int > vertex_offset_shift = FIXED_DEGREE_SHIFT;

    pobj::vector< size_t > edges;
    pobj::vector< dist_t > edge_dist;

    FixedDegreeGraph( pobj::pool_base& pop, size_t num_vertices )
    {
        pobj::transaction::run( pop, [&] {
            edges.resize( ( (size_t) num_vertices ) << vertex_offset_shift );
            edge_dist.resize( ( (size_t) num_vertices ) << vertex_offset_shift );
        } );
    }
};