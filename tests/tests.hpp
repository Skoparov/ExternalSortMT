#ifndef EXTERNAL_SORT_TESTS_HPP
#define EXTERNAL_SORT_TESTS_HPP

#include <cassert>
#include <cstdio>
#include <random>

#include "../external_sort.hpp"

namespace external_sort
{

namespace test_details
{

void throw_assert( bool val, const std::string& error )
{
    if( !val ){
        throw std::runtime_error( error );
    }
}

void generate_corrupted( const std::string& path )
{
    std::ofstream out(path, std::ios::out | std::ofstream::binary);
    throw_assert( out.good(), "generate_corrupted() : stream not good()");

    size_t t = 0;
    out.write((char*)&t, sizeof(short) / 2);
}

std::vector< size_t > generate_file( const std::string& path, size_t number_of_items )
{
    std::vector< size_t > numbers;

    std::random_device r;
    std::default_random_engine e1(r());
    std::uniform_int_distribution<size_t> uniform_dist(1, number_of_items);

    for (size_t i = 0; i < number_of_items; ++i){
        numbers.push_back(uniform_dist(e1));
    }

    // check that file is written
    std::ofstream out(path, std::ios::out | std::ofstream::binary);
    throw_assert( out.good(), "generate_file() : out stream not good()");

    if (number_of_items){
        out.write((char*)&numbers[0], numbers.size() * sizeof(size_t));
    }

    out.close();

    std::vector< size_t > numbers2(numbers.size());
    std::ifstream in(path, std::ios::in | std::ifstream::binary);
    throw_assert( out.good(), "generate_file() : in stream not good()");
    in.read(reinterpret_cast<char*>(&numbers2[0]), numbers.size()*sizeof(size_t));

    throw_assert( numbers == numbers2, "sort data not valid");

    return numbers;
}

bool verify( std::string sorted_file_path, std::vector< size_t >& numbers )
{
    std::vector< size_t > numbers2( numbers.size() );
    std::ifstream in( sorted_file_path, std::ios::in | std::ifstream::binary );
    throw_assert( in.good(), "verify() : in stream not good()" );

    in.read( reinterpret_cast<char*>(&numbers2[0]), numbers.size()*sizeof(size_t) );
    in.close();

    std::sort( numbers.begin(), numbers.end() );
    return numbers == numbers2;
}

}

namespace tests
{

void test_corrupted_file( const std::string& work_folder )
{
    using namespace test_details;
    std::string file_path = work_folder + "external_sort_test_file";

    try{
        generate_corrupted( file_path );
    }
    catch(...)
    {
        std::remove( file_path.c_str() );
        throw;
    }

    bool exception_thrown{ false };

    try{
        external_sort< size_t >( file_path, work_folder, 1000, 5 );
    }
    catch( const std::exception& ){
        exception_thrown = true;
    }

    std::remove( file_path.c_str() );
    throw_assert( exception_thrown, "test_corrupted_file FAILED" );

    std::cout<<"test_corrupted_file PASSED"<<std::endl;
}

void test_invalid_args()
{
    using namespace test_details;
    bool exception_thrown{ false };

    // invalid mem
    try{
        external_sort< size_t >( "test", "test", 3, 5 );
    }
    catch( const std::exception& ){
        exception_thrown = true;
    }

    throw_assert( exception_thrown, "test_invalid_args FAILED : avail memory" );
    exception_thrown = false;

    // invalid merge at once
    try{
        external_sort< size_t >( "test", "test", 1000, 1, std::thread::hardware_concurrency() - 1 );
    }
    catch( const std::exception& ){
        exception_thrown = true;
    }

    throw_assert( exception_thrown, "test_invalid_args FAILED : merge at once" );
    exception_thrown = false;

    // invalid file names
    try{
        external_sort< size_t >( "", "", 1000, 1, std::thread::hardware_concurrency() - 1 );
    }
    catch( const std::exception& ){
        exception_thrown = true;
    }

    throw_assert( exception_thrown, "test_invalid_args FAILED : file names" );

    std::cout<<"test_invalid_args PASSED"<<std::endl;
}

void test_sort( const std::string& work_folder, size_t avail_mem, size_t merge_at_once, size_t threads_num )
{
    using namespace test_details;
    std::string file_path = work_folder + "external_sort_test_file";
    std::string sorted_file_path = work_folder + "sorted";

    std::vector< size_t > data;
    try{
        data = generate_file( file_path, 10000000 );
    }
    catch(...)
    {
        std::remove( file_path.c_str() );
        throw;
    }

    bool exception_thrown{ false };
    std::string exception_str;

    try{
        external_sort< size_t >( file_path, sorted_file_path, avail_mem, merge_at_once, threads_num );
    }
    catch( const std::exception& e )
    {
        exception_str = e.what();
        exception_thrown = true;
    }

    std::remove( file_path.c_str() );

    try
    {
        throw_assert( verify( sorted_file_path, data ), "test_sort FAILED : data invalid"  );
        throw_assert( !exception_thrown, "test_sort FAILED : exception thrown : " + exception_str );
    }
    catch(...)
    {
        std::remove( sorted_file_path.c_str() );
        throw;
    }

    std::remove( sorted_file_path.c_str() );

    std::cout<<"test_sort PASSED"<<std::endl;
}

void run_all_tests( const std::string& work_folder, size_t avail_mem, size_t merge_at_once, size_t threads_num )
{
    std::cout<<"Running all tests..."<<std::endl;

    try
    {
        test_corrupted_file( work_folder );
        test_invalid_args();
        test_sort( work_folder, avail_mem, merge_at_once, threads_num );
    }
    catch( const std::exception& e )
    {
        std::cout<<e.what()<<std::endl;
        return;
    }

    std::cout<<"All tests passed"<<std::endl;
}

}// tests

}// external_sort

#endif
