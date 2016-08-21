#ifndef MULTIPLE_FILE_READER_HPP
#define MULTIPLE_FILE_READER_HPP

#include "file_chunk_reader.hpp"
#include "noexcept_support.hpp"

namespace external_sort
{

namespace file
{

// A wrapper around multiple file_chunk_readers
// to ease the work of merge stage where several files have to be read at once

template< class T >
class multiple_file_reader
{
public:
    explicit multiple_file_reader( size_t simul_readings, size_t block_size );
    multiple_file_reader( const multiple_file_reader& ) = delete;
    multiple_file_reader& operator=( const multiple_file_reader& ) = delete;

    // Had to declare & define move-related stuff myself
    // due to VS2013 bug (doesn't generate them even though it should)
    multiple_file_reader( multiple_file_reader&& other );
    multiple_file_reader& operator=( multiple_file_reader&& other );

    void open( const std::vector< std::string >& files, size_t number  );
    void close();
    std::vector< T > get_next_chunk( size_t reader );
	inline bool reader_completed(size_t reader) const NOEXCEPT;

private:
    std::vector< file_chunk_reader< T > > m_readers;
    size_t m_block_size;
};


///// implementation

template< typename T >
multiple_file_reader< T >::multiple_file_reader( multiple_file_reader&& other ) :
    m_readers( std::move( other.m_readers ) ),
    m_block_size( other.m_block_size )
{

}

template< typename T >
multiple_file_reader< T >& multiple_file_reader< T >::operator=( multiple_file_reader&& other )
{
    m_readers = std::move( other.m_readers );
    m_block_size = other.m_block_size;

    return *this;
}


template< typename T >
multiple_file_reader< T >::multiple_file_reader( size_t simul_readings, size_t block_size ) :
    m_block_size( block_size )
{
    m_readers.resize( simul_readings );
}

template< typename T >
void multiple_file_reader< T >::open( const std::vector< std::string >& files, size_t number  )
{
    if( files.size() > m_readers.size() ){
        throw std::invalid_argument( "Wring number of files to open" );
    }

    for( size_t file = 0; file < number; ++file ){
        m_readers[ file ].open( files[ file ], m_block_size );
    }
}

template< typename T >
void multiple_file_reader< T >::close()
{
    for( auto& r : m_readers ){
        r.close();
    }
}

template< typename T >
std::vector< T > multiple_file_reader< T >::get_next_chunk( size_t reader )
{
    return m_readers[ reader ].get_next_chunk();
}

template< typename T >
inline bool multiple_file_reader< T >::reader_completed(size_t reader) const NOEXCEPT
{
    return m_readers[ reader ].completed();
}

}// file

}// external_sort

#endif
