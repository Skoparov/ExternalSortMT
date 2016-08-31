#ifndef EXTERNAL_SORT_HPP
#define EXTERNAL_SORT_HPP

#include "details/split_sorter.hpp"
#include "details/merge_sorter.hpp"

namespace external_sort
{

template< typename T >
void external_sort( const std::string& in_file,
                    const std::string& out_file,
                    size_t avail_mem,
                    size_t merge_at_once,
                    size_t threads_num = std::thread::hardware_concurrency() - 1 )
{
    // if avail_mem < memory needed to merge 2 files + output buffer
    if( avail_mem < 3 * sizeof( T ) ){
        throw std::invalid_argument( "Not enough memory to sort" );
    }

    if( merge_at_once < 2 ){
        throw std::invalid_argument( "Cannot merge less that two files" );
    }

    if( in_file.empty() || out_file.empty()  ){
        throw std::invalid_argument( "File name should not be empty" );
    }

    if( threads_num == 0 ){
        threads_num = 1;
    }

    std::string work_folder = common::get_folder_from_path( out_file );

    size_t files_num = split::split< T >( in_file, work_folder, avail_mem, threads_num );
    merge::merge< T >( out_file, files_num, merge_at_once, avail_mem, threads_num );
}

}// external_sort

#endif
