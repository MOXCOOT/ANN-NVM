#pragma once

#include <libpmemobj++/container/vector.hpp>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/p.hpp> // p<>
#include <libpmemobj++/p.hpp>
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/pool.hpp>
#include <libpmemobj++/transaction.hpp>

typedef float value_t;
typedef double dist_t;

namespace pobj = pmem::obj;

inline void suppress_unused_pobj_warning() { (void) sizeof( pobj::persistent_ptr< int > ); }

// #define ACC_BATCH_SIZE 4096
#define ACC_BATCH_SIZE 1000000

// for GPU
#define FIXED_DEGREE 31
#define FIXED_DEGREE_SHIFT 5

// for CPU construction
#define SEARCH_DEGREE 15
#define CONSTRUCT_SEARCH_BUDGET 150

enum class DistType
{
    L2,
    NEGATIVE_INNER_PROD,
    NEGATIVE_COSINE,
    BIT_HAMMING
};

class Data;
template < DistType T >
class FixedDegreeGraph;

struct Root
{
    pobj::persistent_ptr< FixedDegreeGraph< DistType::L2 > > graph;
    pobj::persistent_ptr< Data > data;
};

extern pobj::pool< Root > pop;

extern pobj::persistent_ptr< Root > root;
