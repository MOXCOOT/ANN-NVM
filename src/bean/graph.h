#pragma once
#include "../config.h"
#include "data.h"
#include <algorithm>
#include <queue>
#include <unordered_set>

class Graph
{
  public:
    // virtual void add_vertex( size_t vertex_id, std::vector< std::pair< size_t, value_t > >& point ) = 0;
    // virtual void search_top_k( const std::vector< std::pair< size_t, value_t > >& query, int k, std::vector< size_t >& result ) = 0;
    // virtual void search_top_k_batch( const std::vector< std::vector< std::pair< int, value_t > > >& queries, int k, std::vector< std::vector< size_t > >& results ){};
    // virtual ~Graph(){};
};

template < DistType dist_type >
class FixedDegreeGraph : public Graph
{
  public:
    long long total_explore_cnt = 0;
    int total_explore_times = 0;
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

    void search_top_k( const std::vector< std::pair< size_t, value_t > >& query, int k, std::vector< size_t >& result ) { astar_multi_start_search( query, k, result ); }

    template < class T >
    dist_t pair_distance_naive( size_t a, T& b )
    {
        return distance( a, b );
    }

    void astar_multi_start_search( const std::vector< std::pair< size_t, value_t > >& query, int k, std::vector< size_t >& result )
    {
        std::priority_queue< std::pair< dist_t, size_t >, std::vector< std::pair< dist_t, size_t > >, std::greater< std::pair< dist_t, size_t > > > q;
        const int num_start_point = 1; // 3;

        auto converted_query = root->data->organize_point( query );
        std::unordered_set< size_t > visited;
        // std::cout << "[DEBUG] current vertices " << root->data->curr_vertices() << std::endl;
        for ( int i = 0; i < num_start_point && i < root->data->curr_vertices(); ++i )
        {
            auto start = 0; // rand_gen() % data->curr_vertices();
            if ( visited.count( start ) )
                continue;
            visited.insert( start );
            q.push( std::make_pair( pair_distance_naive( start, converted_query ), start ) );
        }
        std::priority_queue< std::pair< dist_t, size_t > > topk;
        const int max_step = 1000000;
        bool found_min_node = false;
        int explore_cnt = 0;

        for ( int iter = 0; iter < max_step && !q.empty(); ++iter )
        {
            auto now = q.top();
            if ( topk.size() == k && topk.top().first < now.first )
            {
                break;
            }
            ++explore_cnt;
            q.pop();
            topk.push( now );
            // std::cout << "[DEBUG] now " << now.first << std::endl;
            // std::cout << "[DEBUG] topk size " << topk.size() << std::endl;
            if ( topk.size() > k )
                topk.pop();
            auto offset = ( (size_t) now.second ) << vertex_offset_shift;

            // std::cout << "[DEBUG] offset=" << offset << " edges.size()=" << edges.size() << std::endl;

            // 范围检查
            assert( offset < edges.size() ); // 如果越界，程序会在这里中断
            if ( offset >= edges.size() )
            {
                std::cerr << "ERROR: edges 越界，offset=" << offset << "，合法范围 [0," << edges.size() - 1 << "]" << std::endl;
                break; // 或者 return，退出循环
            }
            auto degree = edges[offset];
            for ( int i = 0; i < degree; ++i )
            {
                auto start = edges[offset + i + 1];
                if ( visited.count( start ) )
                    continue;
                q.push( std::make_pair( pair_distance_naive( start, converted_query ), start ) );
                auto tmp = pair_distance_naive( start, converted_query );
                visited.insert( start );
            }
        }

        total_explore_cnt += explore_cnt;
        ++total_explore_times;
        result.resize( topk.size() );

        int i = result.size() - 1;
        while ( !topk.empty() )
        {
            result[i] = ( topk.top().second );
            topk.pop();
            --i;
        }
    }
    template < class T >
    dist_t distance( size_t a, T& b )
    {
        auto pa = root->data->get( a );
        dist_t ret = 0;
        for ( int i = 0; i < root->data->get_dim(); ++i )
        {
            auto diff = *( pa + i ) - b[i];
            ret += diff * diff;
        }
        return ret;
    }

    dist_t distance( size_t a, size_t b )
    {
        auto pa = root->data->get( a ), pb = root->data->get( b );
        dist_t ret = 0;
        for ( int i = 0; i < root->data->get_dim(); ++i )
        {
            auto diff = *( pa + i ) - *( pb + i );
            ret += diff * diff;
        }
        return ret;
    }

    void compute_distance( size_t offset, std::vector< dist_t >& dists )
    {
        dists.resize( edges[offset] );
        auto degree = edges[offset];
        for ( int i = 0; i < degree; ++i )
        {
            dists[i] = distance( offset >> vertex_offset_shift, edges[offset + i + 1] );
        }
    }

    void rank_edges( size_t offset )
    {
        std::vector< dist_t > dists;
        compute_distance( offset, dists );
        pobj::transaction::run( pop, [&] {
            for ( int i = 0; i < dists.size(); ++i )
                edge_dist[offset + i + 1] = dists[i];
            std::sort( edge_dist.begin() + offset + 1, edge_dist.begin() + offset + 1 + dists.size() );
        } );
    }

    void rank_and_switch_ordered( size_t v_id, size_t u_id )
    {
        // We assume the neighbors of v_ids in edges[offset] are sorted
        // by the distance to v_id ascendingly when it is full
        // NOTICE: before it is full, it is unsorted
        auto curr_dist = distance( v_id, u_id );
        auto offset = ( (size_t) v_id ) << vertex_offset_shift;
        // We assert edges[offset] > 0 here
        if ( curr_dist >= edge_dist[offset + edges[offset]] )
        {
            //   printf("[DEBUG] skip switch, degree %zu, nodes: ",edges[offset]);
            //   for(int i = 0;i < edges[offset];++i)
            //       printf("(%d,%f) ",)
            return;
        }
        pobj::transaction::run( pop, [&] {
            edges[offset + edges[offset]] = u_id;
            edge_dist[offset + edges[offset]] = curr_dist;
        } );
        for ( size_t i = offset + edges[offset] - 1; i > offset; --i )
        {
            if ( edge_dist[i] > edge_dist[i + 1] )
            {
                pobj::transaction::run( pop, [&] {
                    std::swap( edges[i], edges[i + 1] );
                    std::swap( edge_dist[i], edge_dist[i + 1] );
                } );
            }
            else
            {
                break;
            }
        }
    }

    void add_edge( size_t v_id, size_t u_id )
    {
        auto offset = ( (size_t) v_id ) << vertex_offset_shift;
        if ( edges[offset] < flexible_degree )
        {
            pobj::transaction::run( pop, [&] {
                ++edges[offset];
                edges[offset + edges[offset]] = u_id;
            } );
            if ( edges[offset] == flexible_degree )
            {
                rank_edges( offset );
            }
        }
        else
        {
            rank_and_switch_ordered( v_id, u_id );
        }
    }

    // void add_vertex( size_t vertex_id ) {}

    void add_vertex( size_t vertex_id, std::vector< std::pair< size_t, value_t > >& point )
    {
        // std::cout << "[DEBUG] add vertex " << vertex_id << std::endl;
        // for ( int i = 0; i < 3; ++i )
        // {
        //     std::cout << edges[i] << " ";
        // }
        // std::cout << std::endl;
        std::vector< size_t > neighbor;
        search_top_k( point, CONSTRUCT_SEARCH_BUDGET, neighbor );
        // fprintf(stderr,"[DEBUG] adding %zu, top %d:",vertex_id,degree);
        int num_neighbors = static_cast< int >( degree ) < neighbor.size() ? static_cast< int >( degree ) : neighbor.size();
        // std::cout << "[DEBUG] add vertex " << vertex_id << " with " << num_neighbors << " neighbors" << std::endl;
        auto offset = ( (size_t) vertex_id ) << vertex_offset_shift;
        pobj::transaction::run( pop, [&] {
            edges[offset] = num_neighbors;
            for ( int i = 0; i < neighbor.size() && i < degree; ++i )
            {
                edges[offset + i + 1] = neighbor[i];
            }
        } );
        rank_edges( offset );
        for ( int i = 0; i < neighbor.size() && i < degree; ++i )
        {
            add_edge( neighbor[i], vertex_id );
        }
        root->data->set_curr_vertices( vertex_id + 1 );
    }
    void print_edges( int x )
    {
        for ( int i = 0; i < x; ++i )
        {
            size_t offset = i << vertex_offset_shift;
            int degree = edges[offset];
            fprintf( stderr, "%d (%d): ", i, degree );
            for ( int j = 1; j <= degree; ++j )
                fprintf( stderr, "(%zu,%f) ", edges[offset + j], edge_dist[offset + j] );
            fprintf( stderr, "\n" );
        }
    }
};
