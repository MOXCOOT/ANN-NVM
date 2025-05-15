// #pragma once
// #include <stdlib.h>

// #include <algorithm>
// #include <queue>
// #include <random>
// #include <unordered_set>
// #include <vector>

// #include "../config.h"
// #include "data.h"

// class GraphWrapper
// {
//   public:
//     virtual void
//     add_vertex( size_t vertex_id, std::vector< std::pair< int, value_t > >& point ) = 0;
//     virtual void search_top_k(
//         const std::vector< std::pair< int, value_t > >& query, int k,
//         std::vector< size_t >& result ) = 0;
//     // virtual void search_top_k_batch(const
//     // std::vector<std::vector<std::pair<int, value_t>>> &queries, int k,
//     // std::vector<std::vector<size_t>> &results) {};
//     virtual ~GraphWrapper(){};
// };

// template < DistType dist_type >
// class FixedDegreeGraph : public GraphWrapper
// {
//   private:
//     const int degree = SEARCH_DEGREE;
//     const int flexible_degree = FIXED_DEGREE;
//     const int vertex_offset_shift = FIXED_DEGREE_SHIFT;
//     pobj::persistent_ptr< pobj::vector< size_t > > edges;
//     pobj::persistent_ptr< pobj::vector< dist_t > > edge_dist;
//     pobj::persistent_ptr< Data > data;

//     void rank_and_switch_ordered( size_t v_id, size_t u_id )
//     {
//         // We assume the neighbors of v_ids in edges[offset] are sorted
//         // by the distance to v_id ascendingly when it is full
//         // NOTICE: before it is full, it is unsorted
//         auto curr_dist = pair_distance( v_id, u_id );
//         auto offset = ( (size_t) v_id ) << vertex_offset_shift;
//         // We assert edges[offset] > 0 here
//         if ( curr_dist >= ( *edge_dist )[offset + ( *edges )[offset]] )
//         {
//             return;
//         }
//         ( *edges )[offset + ( *edges )[offset]] = u_id;
//         ( *edge_dist )[offset + ( *edges )[offset]] = curr_dist;
//         for ( size_t i = offset + ( *edges )[offset] - 1; i > offset; --i )
//         {
//             if ( edge_dist[i] > edge_dist[i + 1] )
//             {
//                 std::swap( edges[i], edges[i + 1] );
//                 std::swap( edge_dist[i], edge_dist[i + 1] );
//             }
//             else
//             {
//                 break;
//             }
//         }
//     }

//     void rank_and_switch( size_t v_id, size_t u_id )
//     {
//         rank_and_switch_ordered( v_id, u_id );
//         // TODO:
//         // Implement an unordered version to compare with
//     }

//     template < class T >
//     dist_t distance( size_t a, T& b )
//     {
//         if ( dist_type == DistType::L2 )
//             return data->l2_distance( a, b );
//         else
//         {
//             std::cout << "error distance type" << std::endl;
//             exit( -10 );
//         }
//     }

//     void compute_distance_naive( size_t offset, std::vector< dist_t >& dists )
//     {
//         dists.resize( ( *edges )[offset] );
//         auto degree = ( *edges )[offset];
//         for ( int i = 0; i < degree; ++i )
//         {
//             dists[i] = distance( offset >> vertex_offset_shift, ( *edges )[offset + i + 1] );
//         }
//     }

//     void compute_distance( size_t offset, std::vector< dist_t >& dists )
//     {
//         compute_distance_naive( offset, dists );
//     }

//     template < class T >
//     dist_t pair_distance_naive( size_t a, T& b )
//     {
//         return distance( a, b );
//     }

//     template < class T >
//     dist_t pair_distance( size_t a, T& b )
//     {
//         return pair_distance_naive( a, b );
//     }

//     void qsort( size_t l, size_t r )
//     {
//         auto mid = ( l + r ) >> 1;
//         int i = l, j = r;
//         auto k = edge_dist[mid];
//         do
//         {
//             while ( edge_dist[i] < k )
//                 ++i;
//             while ( k < edge_dist[j] )
//                 --j;
//             if ( i <= j )
//             {
//                 std::swap( edge_dist[i], edge_dist[j] );
//                 std::swap( edges[i], edges[j] );
//                 ++i;
//                 --j;
//             }
//         } while ( i <= j );
//         if ( i < r )
//             qsort( i, r );
//         if ( l < j )
//             qsort( l, j );
//     }

//     void rank_edges( size_t offset )
//     {
//         std::vector< dist_t > dists;
//         compute_distance( offset, dists );
//         for ( int i = 0; i < dists.size(); ++i )
//             ( *edge_dist )[offset + i + 1] = dists[i];
//         qsort( offset + 1, offset + dists.size() );
//         // TODO:
//         // use a heap in the edge_dist
//     }

//     void add_edge( size_t v_id, size_t u_id )
//     {
//         auto offset = ( (size_t) v_id ) << vertex_offset_shift;
//         if ( ( *edges )[offset] < flexible_degree )
//         {
//             ++( *edges )[offset];
//             ( *edges )[offset + ( *edges )[offset]] = u_id;
//             if ( ( *edges )[offset] == flexible_degree )
//             {
//                 rank_edges( offset );
//             }
//         }
//         else
//         {
//             rank_and_switch( v_id, u_id );
//         }
//     }

//   public:
//     long long total_explore_cnt = 0;
//     int total_explore_times = 0;

//     FixedDegreeGraph( pobj::pool_base& pop, pobj::persistent_ptr< Data > data ) : data( data )
//     {
//         pobj::transaction::run( pop, [&] {
//             edges = pobj::make_persistent< pobj::vector< size_t > >(
//                 (size_t) data->max_vertices() << vertex_offset_shift );
//             edge_dist = pobj::make_persistent< pobj::vector< dist_t > >(
//                 (size_t) data->max_vertices() << vertex_offset_shift );
//         } );

//         // pobj::persistent_ptr<pobj::vector<size_t>> edges;
//         // pobj::persistent_ptr<pobj::vector<dist_t>> edge_dist;
//         // pobj::persistent_ptr<Data> data;

//         // auto num_vertices = data->max_vertices();
//         // (*edges) = std::vector<size_t>(((size_t)num_vertices) <<
//         // vertex_offset_shift);
//         // (*edge_dist) = std::vector<dist_t>(((size_t)num_vertices) <<
//         // vertex_offset_shift);
//     }

//     void add_vertex( size_t vertex_id, std::vector< std::pair< int, value_t > >& point )
//     {
//         std::vector< size_t > neighbor;
//         search_top_k( point, CONSTRUCT_SEARCH_BUDGET, neighbor );
//         // fprintf(stderr,"[DEBUG] adding %zu, top
//         // %d:",vertex_id,degree);
//         int num_neighbors = degree < neighbor.size() ? degree : neighbor.size();
//         auto offset = ( (size_t) vertex_id ) << vertex_offset_shift;
//         ( *edges )[offset] = num_neighbors;
//         // TODO:
//         // it is possible to save this space --- edges[offset]
//         // by set the last number in the range as
//         // a large number - current degree
//         for ( int i = 0; i < neighbor.size() && i < degree; ++i )
//         {
//             ( *edges )[offset + i + 1] = neighbor[i];
//         }
//         rank_edges( offset );
//         for ( int i = 0; i < neighbor.size() && i < degree; ++i )
//         {
//             add_edge( neighbor[i], vertex_id );
//         }
//     }
//     void astar_multi_start_search(
//         const std::vector< std::pair< int, value_t > >& query, int k,
//         std::vector< size_t >& result )
//     {
//         std::priority_queue<
//             std::pair< dist_t, size_t >, std::vector< std::pair< dist_t, size_t > >,
//             std::greater< std::pair< dist_t, size_t > > >
//             q;
//         const int num_start_point = 1; // 3;

//         auto converted_query = data->organize_point( query );
//         std::unordered_set< size_t > visited;
//         for ( int i = 0; i < num_start_point && i < data->curr_vertices(); ++i )
//         {
//             auto start = 0; // rand_gen() % data->curr_vertices();
//             if ( visited.count( start ) )
//                 continue;
//             visited.insert( start );
//             q.push( std::make_pair( pair_distance_naive( start, converted_query ), start ) );
//         }
//         std::priority_queue< std::pair< dist_t, size_t > > topk;
//         const int max_step = 1000000;
//         bool found_min_node = false;
//         int explore_cnt = 0;
//         for ( int iter = 0; iter < max_step && !q.empty(); ++iter )
//         {
//             auto now = q.top();
//             if ( topk.size() == k && topk.top().first < now.first )
//             {
//                 break;
//             }
//             ++explore_cnt;
//             q.pop();
//             topk.push( now );
//             if ( topk.size() > k )
//                 topk.pop();
//             auto offset = ( (size_t) now.second ) << vertex_offset_shift;
//             auto degree = ( *edges )[offset];
//             for ( int i = 0; i < degree; ++i )
//             {
//                 auto start = ( *edges )[offset + i + 1];
//                 if ( visited.count( start ) )
//                     continue;
//                 q.push( std::make_pair( pair_distance_naive( start, converted_query ), start ) );
//                 auto tmp = pair_distance_naive( start, converted_query );
//                 visited.insert( start );
//             }
//         }
//         total_explore_cnt += explore_cnt;
//         ++total_explore_times;
//         result.resize( topk.size() );
//         int i = result.size() - 1;
//         while ( !topk.empty() )
//         {
//             result[i] = ( topk.top().second );
//             topk.pop();
//             --i;
//         }
//     }
//     void search_top_k(
//         const std::vector< std::pair< int, value_t > >& query, int k,
//         std::vector< size_t >& result )
//     {
//         astar_multi_start_search( query, k, result );
//     }

//     // void search_top_k_batch(const std::vector<std::vector<std::pair<int,
//     // value_t>>> &queries, int k, std::vector<std::vector<size_t>>
//     // &results)
//     // {
//     //     for (int i = 0; i < queries.size(); ++i)
//     //         search_top_k(queries[i], k, results[i]);
//     // }
// };