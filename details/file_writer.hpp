#ifndef FILE_WRITER_HPP
#define FILE_WRITER_HPP

#include <fstream>
#include <stdexcept>
#include <memory>

namespace external_sort
{

namespace file
{

// JUst a handy wrapper to write data to a file
template< typename T >
class file_writer
{
public:	
    explicit file_writer();

    file_writer( const file_writer& ) = delete;
    file_writer& operator=( const file_writer& ) = delete;

    // Had to declare & define move-related stuff myself
    // due to VS2013 bug (doesn't generate them even though it should)
    file_writer( file_writer&& other );
    file_writer& operator=( file_writer&& other );

    void open( const std::string& out_file );
    void write( std::vector< T > data );
    void close();

private:
    std::unique_ptr< std::ofstream > m_out;
};

///// implementation

template< typename T >
file_writer< T >::file_writer( file_writer&& other ) : m_out( std::move( other.m_out ) )
{

}

template< typename T >
file_writer< T >& file_writer< T >::operator=( file_writer&& other )
{
    m_out = std::move( other.m_out );
    return *this;
}

template< typename T >
file_writer< T >::file_writer() : m_out( std::unique_ptr< std::ofstream >{ new std::ofstream() } )
{

}

template< typename T >
void file_writer< T >::open( const std::string& out_file )
{
    m_out->open( out_file, std::ios::out | std::ofstream::binary );
    if( !m_out->good() ){
        throw std::runtime_error{ "Couldn't write to file: " + out_file };
    }
}

template< typename T >
void file_writer< T >::close()
{
    m_out->close();
}

template< typename T >
void file_writer< T >::write( std::vector< T > data )
{
     m_out->write( ( char* )&data[0], data.size() * sizeof( T ) );
}

}// file

}// external_sort

#endif
