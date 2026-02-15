cmake_minimum_required(VERSION 3.29)
project(nexus
        LANGUAGES CXX
        VERSION 1.0.0
)

option(NEXUS_ENABLE_TEST "Enable the test target" OFF)
option(NEXUS_USE_STD_TYPE_INDEX "Use std::type_index instead of a tag member" OFF)

add_library(nexus INTERFACE)
add_library(nexus::nexus ALIAS nexus)

target_sources(
        nexus

        PUBLIC
        FILE_SET nexus_headers
        TYPE HEADERS
        BASE_DIRS include
        FILES
        "include/nexus/nexus.hpp"
)

target_compile_features(nexus INTERFACE cxx_std_20)

if (NEXUS_USE_STD_TYPE_INDEX)
    target_compile_definitions(nexus INTERFACE NEXUS_USE_STD_TYPE_INDEX)
endif ()

if (NEXUS_ENABLE_TEST)
    include(CTest)

    add_executable(test_basic test/test_basic.cpp)
    target_link_libraries(test_basic PRIVATE nexus::nexus)
    target_compile_features(test_basic PRIVATE cxx_std_20)

    add_executable(test_multiple test/test_multiple.cpp)
    target_link_libraries(test_multiple PRIVATE nexus::nexus)
    target_compile_features(test_multiple PRIVATE cxx_std_20)

    add_executable(test_capture test/test_capture.cpp)
    target_link_libraries(test_capture PRIVATE nexus::nexus)
    target_compile_features(test_capture PRIVATE cxx_std_20)

    add_test(NAME test_basic COMMAND $<TARGET_FILE:test_basic>)
    add_test(NAME test_capture COMMAND $<TARGET_FILE:test_capture>)
    add_test(NAME test_multiple COMMAND $<TARGET_FILE:test_multiple>)
endif ()