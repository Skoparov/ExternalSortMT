#ifndef FILE_PART_HPP
#define FILE_PART_HPP

#include <functional>
#include <vector>

namespace external_sort
{

namespace merge
{

// A wrapper around a file being read during the merge
template< class T >
class file_part
{
public:
    void clear();
    void update_data( std::vector< T >&& d );
	void set_file_index(int index) NOEXCEPT;
    const T& peek_next() const;
    const T& next() const;
	inline bool empty() const NOEXCEPT;
	inline int file_index() const NOEXCEPT;
	inline bool finished() const NOEXCEPT;

private:
    std::vector< T > m_data;
    int m_file_index{ -1 };
    mutable size_t m_iterator{ 0 };
};

///// implementation

template< class T >
void file_part< T >::clear()
{
    m_data.clear();
    m_iterator = 0;
}

template< class T >
void file_part< T >::update_data( std::vector< T >&& d )
{
    m_data = std::move( d );
}

template< class T >
void file_part< T >::set_file_index( int index ) NOEXCEPT
{
    m_file_index = index;
}

template< class T >
const T& file_part< T >::peek_next() const
{
    if( m_data.empty() ){
        throw std::out_of_range{ "Part is empty" };
    }

    return m_data[ m_iterator ];
}

template< class T >
const T& file_part< T >::next() const
{
    if( m_data.empty() ){
        throw std::out_of_range{ "Part is empty" };
    }

    return m_data[ m_iterator++ ];
}

template< class T >
inline bool file_part< T >::empty() const NOEXCEPT
{
    return m_data.empty();
}

template< class T >
inline int file_part< T >::file_index() const NOEXCEPT
{
    return m_file_index;
}

template< class T >
inline bool file_part< T >::finished() const NOEXCEPT
{
    return m_iterator == m_data.size();
}

} // sort

} //external_sort

#endif
