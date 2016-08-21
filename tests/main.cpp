#include "tests.hpp"
#include "../external_sort.hpp"

int main(int argc, char* argv[])
{   
    if ( argc == 2 )
    {
        std::string work_folder{ argv[ 1 ] };
        size_t mem = 1000000;
        size_t merge_at_once = 5;		
        external_sort::tests::run_all_tests( work_folder, mem, merge_at_once, std::thread::hardware_concurrency() - 1 );
    }
    else{
        std::cout<<"Should provide path to directory WITH TRAILING SLASHES to run tests"<<std::endl;;
    }

    return 0;
}


