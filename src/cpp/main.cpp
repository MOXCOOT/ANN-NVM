#include "../config.h"

#include "../bean/data.h"
#include "../bean/graph.h"
#include "../bean/parser.h"

#include <iostream>
#include <vector>

struct Root
{
    pobj::persistent_ptr< Data > data;
    pobj::persistent_ptr< FixedDegreeGraph< DistType::L2 > > graphobj;
};
pobj::pool< Root > pop;

pobj::persistent_ptr< Data > dataobj;
pobj::persistent_ptr< FixedDegreeGraph< DistType::L2 > > graphobj;

void build_callback( size_t idx, std::vector< std::pair< size_t, value_t > > point ) { dataobj->add( pop, idx, point ); }

int main( int argc, char* argv[] )
{
    const char* path_pmem = argv[1];
    const char* path_data = argv[2];
    const size_t pool_size = 1024 * 1024 * 10; // 10MB
    const size_t n = 15000;
    const int dim = 16;
    int exist_pool = 0;

    if ( pmem::obj::pool< Root >::check( path_pmem, "DATA_LAYOUT" ) != 1 )
    {
        std::cout << "Creating new pool..." << std::endl;
        pop = pobj::pool< Root >::create( path_pmem, "DATA_LAYOUT", pool_size, S_IWUSR | S_IRUSR );
        // 在事务中分配Data对象
        pobj::transaction::run( pop, [&] {
            dataobj = pobj::make_persistent< Data >( pop, n, dim );
            graphobj = pobj::make_persistent< FixedDegreeGraph< DistType::L2 > >( pop, dataobj );
        } );
    }
    else
    {
        exist_pool = 1;
        std::cout << "Opening existing pool..." << std::endl;
        pop = pobj::pool< Root >::open( path_pmem, "DATA_LAYOUT" );
    }

    dataobj = pop.root()->data;
    graphobj = pop.root()->graphobj;
    if ( !exist_pool )
        std::unique_ptr< LibSVMParser > build_parser( new LibSVMParser( path_data, build_callback ) );
    else
        dataobj->print();
    pop.close(); // 关闭持久化内存池
    return 0;
}