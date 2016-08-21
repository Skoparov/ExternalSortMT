project( external_sort )
cmake_minimum_required(VERSION 3.5)
add_definitions("-std=c++11")
SET(CMAKE_BUILD_TYPE Debug)
find_package( Threads )

set( SRC
     ../external_sort.hpp
     ../details/file_chunk_reader.hpp
     ../details/file_writer.hpp
     ../details/multiple_file_reader.hpp
     ../details/split_sorter.hpp
     ../details/merge_sorter.hpp
     ../details/file_part.hpp
     ../details/common.hpp
     ../details/async.hpp
     ../details/async.cpp
	 ../details/noexcept_support.hpp
     tests.hpp
     main.cpp)


add_executable( ${PROJECT_NAME} ${SRC} )
target_link_libraries( ${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} )
