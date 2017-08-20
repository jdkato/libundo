cmake_minimum_required(VERSION 3.3)

project(libundo)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)

if($ENV{TRAVIS})
    if($ENV{TRAVIS} STREQUAL "true")
        message(STATUS "Building on Travis-CI.")
        set(IS_TRAVIS_BUILD TRUE)
    endif()
endif()

option(GENERATE_COVERAGE_INFO "If set, line coverage info will be generated from debug test runs." OFF)
if(GENERATE_COVERAGE_INFO)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # this is an undocumented option for changing the output extension from *.cpp.ext
        # to *.ext; As there is no reasonable way to obtain the list of object files
        # this seems to be the best option to generate the naming gcov expects.
        set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)
        set(GCOV_COMMAND gcov CACHE STRING "Command to invoke to run gcov.")
    endif()
endif()

###############################################################################
# Library details
###############################################################################

set(BASE_VERSION_MAJOR 0)
set(BASE_VERSION_MINOR 1)
set(BASE_VERSION_PATCH 0)
set(BASE_VERSION "${BASE_VERSION_MAJOR}.${BASE_VERSION_MINOR}.${BASE_VERSION_PATCH}")

set(BASE_TEST_SOURCES
    TestBase.cc
    TestTree.cc
    TestFile.cc
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(../include)

###############################################################################
# Testing
###############################################################################

option(BUILD_TESTS "Determines whether to build tests." ON)
if(BUILD_TESTS)
    enable_testing()

    if(NOT TARGET Catch)
        include(ExternalProject)
        if(IS_TRAVIS_BUILD)
            set(FETCH_EXTERNAL_CATCH
                GIT_REPOSITORY https://github.com/philsquared/Catch.git
                GIT_TAG 0c1c9fa9229c7358ce16c9910004a39290b20f5f)
        elseif(WIN32)
            set(FETCH_EXTERNAL_CATCH
                URL https://github.com/philsquared/Catch/archive/v1.2.1-develop.12.zip
                URL_HASH MD5=cda228922a1c9248364c99a3ff9cd9fa)
        else()
            set(FETCH_EXTERNAL_CATCH
                URL https://github.com/philsquared/Catch/archive/v1.2.1-develop.12.tar.gz
                URL_HASH MD5=a8dfb7be899a6e7fb30bd55d53426122)
        endif()
        ExternalProject_Add(Catch-External
            PREFIX ${CMAKE_BINARY_DIR}/Catch
            ${FETCH_EXTERNAL_CATCH}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/Catch/src/Catch-External/single_include/catch.hpp
                                                     ${CMAKE_BINARY_DIR}/Catch/include/catch.hpp
        )
        add_library(Catch INTERFACE)
        add_dependencies(Catch Catch-External)
        target_include_directories(Catch INTERFACE ${CMAKE_BINARY_DIR}/Catch/include)
    endif()

    add_executable(run_tests ${BASE_TEST_SOURCES})
    target_link_libraries(run_tests Catch)
    add_test(NAME TestBase COMMAND run_tests)

    if(GENERATE_COVERAGE_INFO AND (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
        target_compile_options(libundo PRIVATE --coverage -fprofile-arcs -ftest-coverage)
        target_link_libraries(libundo PRIVATE -fprofile-arcs -ftest-coverage)
        add_custom_target(libundo_TestCoverage
            COMMAND ${GCOV_COMMAND} -o CMakeFiles/libundo.dir/src ${BASE_SOURCE_FILES}
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
            COMMENT "Collecting gcov coverage information"
        )
    endif()
endif()
