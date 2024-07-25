add_subdirectory(trompeloeil)

find_package(Catch2 3 REQUIRED)
if(NOT Catch2_FOUND)
    add_subdirectory(catch2)
endif()

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

add_executable(test_slipc test_slipc.cc)
target_link_libraries(test_slipc PRIVATE Catch2::Catch2WithMain)
target_link_libraries(test_slipc PRIVATE slipc)
catch_discover_tests(test_slipc)
