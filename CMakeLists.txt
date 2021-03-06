# Copyright (c) 2018 Weiming Hu

cmake_minimum_required (VERSION 3.1)
project (Matrix)
set (CMAKE_CXX_STANDARD 11)

option(PROFILE_TIME "Build programs with profiling functions" OFF)
option(WALL_TIME "Build programs with profiling functions for wall time" OFF)
option(USE_MPI "Build programs with USE_MPI" OFF)

if (${WALL_TIME})
    add_definitions(-D_WALL_TIME)
    message(STATUS "Build programs with profiling functions for wall time")
endif (${WALL_TIME})

if (${PROFILE_TIME})
    add_definitions(-D_PROFILE_TIME)
    message(STATUS "Build programs with profiling functions")
endif (${PROFILE_TIME})

if (${USE_MPI})
    add_definitions(-D_USE_MPI)
    message(STATUS "Build programs with MPI")
    find_package(MPI REQUIRED)
    include_directories(${MPI_INCLUDE_PATH})
    message(STATUS "Include MPI hear path: ${MPI_INCLUDE_PATH}")
endif (${USE_MPI})

# find OpenMP
find_package (OpenMP)
if (${OPENMP_FOUND})
    message(STATUS "Using OpenMP ${OpenMP_CXX_FLAGS}")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
else (${OPENMP_FOUND})
    message (WARNING "No OpenMP support found for the compiler: ${CMAKE_CXX_COMPILER}
    The process goes on, but the executable won't support multi-threading.
    SUGGESTION: specify another compiler using CC and CXX. CC=[C compiler] CXX=[CXX compiler] cmake ..")
endif (${OPENMP_FOUND})

if (CMAKE_BUILD_TYPE)
    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        message(STATUS "Add definitions for debugging")
        add_definitions(-D_DEBUG)
    endif (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
endif (CMAKE_BUILD_TYPE)

# Set the output folder
set (COMMON_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/output")
if (EXISTS ${COMMON_OUTPUT_DIR})
    file(REMOVE_RECURSE ${COMMON_OUTPUT_DIR})
endif(EXISTS ${COMMON_OUTPUT_DIR})
file(MAKE_DIRECTORY ${COMMON_OUTPUT_DIR})

# Add the library Matrix
set (Matrix_SOURCES "src/Matrix.cpp")
include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/src")
#add_library (Matrix SHARED ${Matrix_SOURCES})
add_library (Matrix STATIC ${Matrix_SOURCES})

set_target_properties(Matrix
    PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${COMMON_OUTPUT_DIR}/lib")

# Set the names of the files for building executables
set (PROGRAM_NAMES "testMatrix;directSolver;iterativeSolver")
foreach (PROGRAM_NAME IN LISTS PROGRAM_NAMES)
    message(STATUS "building program ${PROGRAM_NAME}")
    add_executable (${PROGRAM_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/src/${PROGRAM_NAME}.cpp")
    target_link_libraries (${PROGRAM_NAME} Matrix)
    set_target_properties(${PROGRAM_NAME}
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${COMMON_OUTPUT_DIR}/bin")

    if (EXE_SUFFIX)
        set_target_properties(${PROGRAM_NAME}
            PROPERTIES SUFFIX ${EXE_SUFFIX})
    endif (EXE_SUFFIX)

    add_dependencies(${PROGRAM_NAME} Matrix)
endforeach(PROGRAM_NAME IN PROGRAM_NAMES)

if (${USE_MPI})

    # Build parallel Jacobi version 1
    add_definitions(-D_USE_MPI)
    message(STATUS "Build parallel Jacobi method with MPI")
    find_package(MPI REQUIRED)
    include_directories(SYSTEM ${MPI_INCLUDE_PATH})
    message(STATUS "Include MPI head path: ${MPI_INCLUDE_PATH}")
    
    message(STATUS "building program parallelJacobi")
    add_executable (parallelJacobi "${CMAKE_CURRENT_SOURCE_DIR}/src/parallelJacobi.cpp")
    target_link_libraries (parallelJacobi Matrix ${MPI_C_LIBRARIES} ${MPI_CXX_LIBRARIES})

    set_target_properties(parallelJacobi
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${COMMON_OUTPUT_DIR}/bin")

    if (EXE_SUFFIX)
        set_target_properties(parallelJacobi
            PROPERTIES SUFFIX ${EXE_SUFFIX})
    endif (EXE_SUFFIX)

    add_dependencies(parallelJacobi Matrix)


    # Build parallel Jacobi version 2
    set(EXTRA_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_MODULE_PATH})
    find_package(NetCDF)
    if (${NETCDF_FOUND})
        message(STATUS "NetCDF is found. Build parallel Jacobi version 2.")

        include_directories(SYSTEM ${NETCDF_INCLUDES})
        message(STATUS "NetCDF include dir: ${NETCDF_INCLUDES}")
        message(STATUS "NetCDF link libraries: ${NETCDF_LIBRARIES}")

        add_executable (parallelJacobi2 "${CMAKE_CURRENT_SOURCE_DIR}/src/parallelJacobi_v2.cpp")
        target_link_libraries (parallelJacobi2 ${NETCDF_LIBRARIES} ${MPI_C_LIBRARIES} ${MPI_CXX_LIBRARIES})
        set_target_properties(parallelJacobi2
            PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${COMMON_OUTPUT_DIR}/bin")

        if (EXE_SUFFIX)
            set_target_properties(parallelJacobi2 PROPERTIES SUFFIX ${EXE_SUFFIX})
        endif (EXE_SUFFIX)

    else (${NETCDF_FOUND})
        message(STATUS "NetCDF is not found. Parallel Jacobi version 2 is not built.")
    endif (${NETCDF_FOUND})

endif (${USE_MPI})

message (STATUS "compile flags are ${CMAKE_CXX_FLAGS}")
