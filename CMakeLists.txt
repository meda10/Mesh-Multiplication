cmake_minimum_required(VERSION 3.17)
project(PRL_2)

set(CMAKE_CXX_STANDARD 14)

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

#add_definitions(-DOMPI_SKIP_MPICXX)

add_executable(PRL_2 mm.cpp)

#target_link_libraries(PRL_2 PRIVATE MPI_C)
target_link_libraries(PRL_2 ${MPI_LIBRARIES})

if(MPI_COMPILE_FLAGS)
    set_target_properties(PRL_2 PROPERTIES
            COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
    set_target_properties(PRL_2 PROPERTIES
            LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()
