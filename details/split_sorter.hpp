#ifndef PARTIAL_SORTER_HPP
#define PARTIAL_SORTER_HPP

#include <algorithm>
#include <iostream>
#include <list>

#include "file_chunk_reader.hpp"
#include "async.hpp"
#include "common.hpp"

namespace external_sort
{

namespace split_details
{

// A single task run concurrently.
// Just read a chunk of input file, sorts it and writes back to disc

template< class T >
void run( std::vector< T > data, const std::string& file_name );

// Remove finished tasks to free memory
void clearFinishedTasks( std::list< concurrency::Task >& tasks, bool waitForRunning = false );

} //split_details


namespace split
{

template< typename T >
size_t split( const std::string& file_path, size_t avail_mem, size_t threads_num )
{
    // calc block size( number of items to read at once )
    size_t block_size = avail_mem / ( threads_num * sizeof( T ) );

    if( block_size < sizeof( T ) ){
        throw std::runtime_error( "Not enough memory to process the specified type with current settings" );
    }

    block_size -= block_size % sizeof( T );

    file::file_chunk_reader< T > reader;
    reader.open( file_path, block_size);

    concurrency::async async( threads_num );

    std::string work_folder = common::get_folder_from_path( file_path );
    std::list< concurrency::Task > tasks;

    size_t total_started = 0;

    // loop starting the workers
    while( !reader.completed() )
    {
        // don't get new chunks until the prev ones are processed!
        async.wait_for_first_vacant();

        // remove the finished tasks to provide memory for new chunks
        split_details::clearFinishedTasks( tasks );

        auto data = reader.get_next_chunk();
        if( data.empty() ){
            break;
        }

        std::string file_name = common::temp_file_path( work_folder, ++total_started );
        auto sort_func = std::bind( &split_details::run< T >, std::move( data ), file_name );

		tasks.emplace_back();
        concurrency::Task& curr = tasks.back();
        curr.task = std::move( std::packaged_task< void() >{ sort_func } );
        curr.result = std::move( async.run( curr.task ) );
    }

    split_details::clearFinishedTasks( tasks, true );

    return total_started;
}

} //split

namespace split_details
{

template< class T >
void run( std::vector< T > data, const std::string& file_name )
{
    std::sort( data.begin(), data.end() );

    std::ofstream out( file_name, std::ios::out | std::ofstream::binary );
    if( !out.good() ){
        throw std::runtime_error( "Coundn't write to file: " + file_name );
    }

    out.write((char*)&data[0], data.size() * sizeof( T ));
    out.close();
}

void clearFinishedTasks( std::list< concurrency::Task >& tasks, bool waitForRunning )
{
    tasks.remove_if([ waitForRunning ]( concurrency::Task& t )
    {
        if( t.result.wait_for(std::chrono::seconds(0)) == std::future_status::ready || waitForRunning )
        {
            t.result.get(); // rethrow
            return true;
        }

        return false;
    });
}

} //split_details

} //external_sort

#endif
