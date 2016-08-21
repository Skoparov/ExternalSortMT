#ifndef DETAILS_HPP
#define DETAILS_HPP

#include <string>

namespace external_sort
{

namespace common
{

std::string get_folder_from_path( const std::string& file_path )
{
    #ifdef __linux__
        std::string delimeter("/");
    #elif _WIN32
        std::string delimeter("\\");
    #else
    #error Platform not supported
    #endif

    size_t last_delim = file_path.find_last_of( delimeter );
    return file_path.substr( 0, last_delim + delimeter.length() );
}

std::string temp_file_path( const std::string& work_folder, size_t file_index )
{
    return work_folder + "_temp_"+ std::to_string( file_index );
}

}// details

}// external_sort

#endif
