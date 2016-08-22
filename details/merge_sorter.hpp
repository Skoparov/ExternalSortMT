#ifndef MERGE_SORTER_HPP
#define MERGE_SORTER_HPP

#include <sstream>
#include <list>

#include "multiple_file_reader.hpp"
#include "file_writer.hpp"
#include "file_part.hpp"
#include "async.hpp"
#include "common.hpp"

namespace external_sort
{

namespace merge_details
{

using namespace external_sort::merge;

template< typename T >
using file_parts = std::vector< file_part< T > >;

using strings = std::vector< std::string >;
using std::string;

template< typename T >
struct io_handler;

template<typename T >
class out_buffer;

string thread_id();

// Renames files a thread wants to claim with it's own temp names
void rename_input_files( size_t first, size_t last, const string& folder, const strings& temp_names );

// Renames a thread's output file to put it back into work queue
void rename_out_file( const string& out_file, const string& folder, size_t files_num );

void remove_files( const strings& files );

// Claims files for a thread to prevent them from being used by another thread
size_t lock_next_files( const strings& temp_names, const string& folder, size_t& files_num, size_t simul_merge );

// Mergesort files into one output file
template< typename T >
void mergesort_parts( file_parts< T >& parts, out_buffer< T >& out, size_t files_merged, io_handler< T >& h );

// The loop threads run while mergesoring files
template< typename T >
void run( io_handler< T >& h, const string& folder, size_t& files_num, size_t simul_merge, size_t buff_size );

} //merge_details

namespace merge
{

template< typename T >
void merge( const std::string& out_file, size_t files_num, size_t simul_merge, size_t avail_mem, size_t threads )
{
    // calc buffer in bytes
    size_t buffer_size = avail_mem / (  simul_merge * threads );

    if( buffer_size < sizeof( T ) ){
         throw std::runtime_error( "Not enough memory to merge specified number of files simultaneously" );
    }

    buffer_size -= buffer_size % sizeof( T );

    // create readers &  writers
    std::vector< merge_details::io_handler< T > > io_handlers;
    std::mutex io_mutex;

    for( size_t reader_ind = 0; reader_ind < threads; ++reader_ind ){			
        io_handlers.emplace_back( simul_merge, buffer_size / sizeof(T), io_mutex );
    }

    std::string folder = common::get_folder_from_path( out_file );
    std::list< concurrency::Task > tasks;
    concurrency::async async( threads );

    // start threads
    for( size_t thread_id = 0; thread_id < threads; ++thread_id )
    {
        auto merge_func = std::bind( &merge_details::run< T >,
                                     std::ref( io_handlers[ thread_id ] ),
                                     folder,
                                     std::ref( files_num ),
                                     simul_merge,
                                     buffer_size );

        tasks.emplace_back();
        concurrency::Task& curr = tasks.back();
        curr.task = std::move( std::packaged_task< void() >{ merge_func } );
        curr.result = std::move( async.run( curr.task ) );
    }

    // wait for finish
    for (auto& t : tasks){
        t.result.get();
    }

    // rename last output file to "sorted"
    std::string out_file_name = common::temp_file_path( folder, 1 );
    std::string result_file_name = folder + "sorted";
    std::rename( out_file_name.c_str(), result_file_name.c_str() );
}

} //split

namespace merge_details
{

// Handles io stuff for a thread
template< typename T >
struct io_handler
{
    io_handler(  size_t in_number, size_t block_size, std::mutex& m ) :
        reader( in_number, block_size ),
        io_mutex( m ){}

    io_handler( const io_handler& ) = delete;
    io_handler& operator=( const io_handler& ) = delete;

    // Had to declare & define move-related stuff myself
    // due to VS2013 bug (doesn't generate them even though it should)
    io_handler( io_handler&& other ) :
        reader( std::move( other.reader ) ),
        writer( std::move( other.writer ) ),
        io_mutex( other.io_mutex )
	{

	}

    io_handler& operator=( io_handler&& other )
	{
		reader = std::move(other.reader);
		writer = std::move(other.writer);
		io_mutex = other.io_mutex;

		return *this;
	}

    file::multiple_file_reader< T > reader;
    file::file_writer< T > writer;
    std::mutex& io_mutex;
};

// A handy wrapper around output buffer used by a thread
template<typename T >
class out_buffer
{
public:
    out_buffer( size_t max_size ) : m_max_size( max_size ){}

    void add( const T& data ){
        m_buffer.emplace_back( data );
    }

    std::vector< T >& data(){
        return m_buffer;
    }

	inline size_t size() const NOEXCEPT{
        return m_buffer.size();
    }

	inline bool full() const NOEXCEPT{
        return m_buffer.size() * sizeof( T ) == m_max_size;
    }
private:
    std::vector< T > m_buffer;
    size_t m_max_size;
};

std::string thread_id()
{
    std::ostringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

void rename_input_files( size_t first, size_t last, const std::string& folder, const strings& temp_names )
{
    for( size_t file = first; file < last; ++file  )
    {
        std::rename( common::temp_file_path( folder, file ).c_str(),
                     temp_names[ file - first ].c_str() );
    }
}

void rename_out_file( const std::string& out_file, const std::string& folder, size_t files_num )
{
    std::string next_file = common::temp_file_path( folder, files_num );
    std::rename( out_file.c_str(), next_file.c_str() );
}

void remove_files( const strings& files )
{
    for( auto& file : files ){
        std::remove( file.c_str() );
    }
}

size_t lock_next_files( const strings& temp_names, const std::string& folder, size_t& files_num, size_t simul_merge )
{
    size_t files_merged = std::min( files_num, simul_merge );
    files_num -= files_merged;

    size_t first_file = files_num + 1;
    rename_input_files( first_file, first_file + files_merged, folder, temp_names );

    return files_merged;
}

template< typename T >
void mergesort_files( file_parts< T >& parts, out_buffer< T >& out, size_t files_merged, io_handler< T >& h )
{
    // read first values
    for( size_t file = 0;  file < files_merged; ++file )
    {
        auto& part = parts[ file ];
        part.update_data( h.reader.get_next_chunk( file ) );
        part.set_file_index( file );
    }

    // while there's data in the files being merged
    while( true )
    {
        // find current min
        file_part< T >* min_part{ nullptr };
        for( auto& p : parts )
        {
            if( !p.finished() && ( !min_part || min_part->peek_next() > p.peek_next() ) ){
                min_part = &p;
            }
        }

        if( min_part )
        {
            out.add( min_part->next() );

            // fill file buffer if depleted
            if( min_part->finished() )
            {
                min_part->clear();
                min_part->update_data( h.reader.get_next_chunk( min_part->file_index() ) );
            }

            // write buffer if necessary
            if( out.full() ){
                h.writer.write( std::move( out.data() ) );
            }
        }
        else{
            break; // files have been fully processed
        }
    }

    // flush buffer if necessary
    if( out.size() ){
        h.writer.write( std::move( out.data() ) );
    }
}

template< typename T >
void run( io_handler< T >& h, const std::string& folder, size_t& files_num, size_t simul_merge, size_t buff_size )
{
    file_parts< T > parts( simul_merge );
    out_buffer< T > out_buff( buff_size );

    // define temp names for files this thread will work with
    std::string in_file_pattern{ folder +  thread_id() + "_thread_temp_" };
    std::string out_file{ folder + "temp_out_" +  thread_id() };

    strings temp_file_names;
    for( size_t file_id = 0; file_id < simul_merge; ++ file_id ){
        temp_file_names.emplace_back( in_file_pattern + std::to_string( file_id ) );
    }

    // loop over files
    while( true )
    {
        size_t files_this_iteration;

        {
            std::lock_guard< std::mutex > l{ h.io_mutex };
            files_this_iteration = lock_next_files( temp_file_names, folder, files_num, simul_merge );
        }

        // open writer and reader
        h.writer.open( out_file );
        h.reader.open( temp_file_names, files_this_iteration );

        // loop over files until empty, filling out buffer with sorted sequence
        mergesort_files( parts, out_buff, files_this_iteration, h );

        h.writer.close();
        h.reader.close();

        std::lock_guard< std::mutex > l{ h.io_mutex };

        // remove processed empty files
        remove_files( temp_file_names );

        // rename merged file to put it back into work
         ++files_num;
        rename_out_file( out_file, folder, files_num );

        // exit if there's only one file left atm
        if( files_num == 1 )
        {
            std::remove( out_file.c_str() );
            break;
        }
    }
}

} //merge_details

} //external_sort

#endif
