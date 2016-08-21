#ifndef FILE_CHUNK_READER_HPP
#define FILE_CHUNK_READER_HPP

#include <fstream>
#include <stdexcept>
#include <memory>
#include <vector>
#include "noexcept_support.hpp"

namespace external_sort
{

namespace file
{

// Reads files by chunks of items
template< class T >
class file_chunk_reader
{
public:
    using value_type = std::vector< T >;

public:  
    explicit file_chunk_reader();
    file_chunk_reader( const file_chunk_reader& ) = delete;
    file_chunk_reader& operator=( const file_chunk_reader& ) = delete;

    // Had to declare & define move-related stuff myself
    // due to VS2013 bug (doesn't generate them even though it should)
    file_chunk_reader( file_chunk_reader&& other );
    file_chunk_reader& operator=( file_chunk_reader&& other );

    void open( const std::string& file_path, size_t block_size );
    void close();
    const value_type get_next_chunk();
    inline bool completed() const NOEXCEPT;

private:
    const std::vector< T > read( size_t number_of_items );

private:
    std::unique_ptr< std::ifstream > m_in;
    size_t m_block_size{ 0 }; // number of items read at once
    bool m_completed{ false };
};

///// implementation

template< typename T >
file_chunk_reader< T >::file_chunk_reader( file_chunk_reader&& other ) :
    m_in( std::move( other.m_in ) ),
    m_block_size( other.m_block_size ),
    m_completed( other.m_completed )
{

}

template< typename T >
file_chunk_reader< T >& file_chunk_reader< T >::operator=( file_chunk_reader&& other )
{
    m_in = std::move( other.m_in );
    m_block_size = other.m_block_size;
    m_completed = other.m_completed;

    return *this;
}


template< class T >
file_chunk_reader< T >::file_chunk_reader() :
    m_in( std::unique_ptr< std::ifstream >{ new std::ifstream() } )
{

}

template< class T >
const std::vector< T > file_chunk_reader< T >::get_next_chunk()
{
    return read( m_block_size );
}

template< class T >
void file_chunk_reader< T >::open( const std::string& file_path, size_t block_size )
{
    m_block_size = block_size;
    m_completed = false;

    if( m_in->is_open() ){
        m_in->close();
    }

    m_in->open( file_path, std::ios::in | std::ifstream::binary | std::ios::ate );

    if( !m_in->good() )
    {
        throw std::invalid_argument(
                    file_path + " doesn't exist or occupied by another process" );
    }

    // cals size
    auto file_size = m_in->tellg();
    m_in->seekg( 0, m_in->beg );

    if( file_size % sizeof( T ) )
    {
        throw std::length_error{
                    file_path + " has size incompatible with the specified type or is corrupted" };
    }
}

template< class T >
void file_chunk_reader< T >::close(  )
{
   m_in->close();
}

template< class T >
const std::vector< T > file_chunk_reader< T >::read( size_t number_of_items )
{
    std::vector< T > result;    

    if( !m_completed )
    {
        result.resize( number_of_items );
        m_in->read( reinterpret_cast< char* >( &result[0] ), number_of_items * sizeof( T ) );
        result.resize( m_in->gcount() / sizeof( T ) ); // resize if red less numbers that specified

        if( m_in->eof() ){
            m_completed = true;
        }
    }

    return result;
}

template< class T >
inline bool file_chunk_reader< T >::completed() const NOEXCEPT
{
    return m_completed;
}

}// file

}// external_sort

#endif
