add_subdirectory(catch2)
add_subdirectory(trompeloeil)

add_executable(test_slipc test_slipc.cc)
target_link_libraries(test_slipc PRIVATE Catch2::Catch2WithMain trompeloeil::trompeloeil)
target_link_libraries(test_slipc PRIVATE slipc)
catch_discover_tests(test_slipc)
