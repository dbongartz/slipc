add_subdirectory(catch2)
add_subdirectory(trompeloeil)

add_executable(test_slipc test_slipc.cc)
target_link_libraries(test_slipc PRIVATE Catch2::Catch2WithMain)
target_link_libraries(test_slipc PRIVATE slipc)
catch_discover_tests(test_slipc)


add_library(callsit callsit.c)
target_include_directories(callsit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_executable(test_tromp test_tromp.cc)
target_link_libraries(test_tromp PRIVATE Catch2::Catch2WithMain trompeloeil::trompeloeil)
target_link_libraries(test_tromp PRIVATE callsit)
catch_discover_tests(test_tromp)


add_subdirectory(doctest)

add_executable(test_d_slipc test_d_slipc.cc)
target_link_libraries(test_d_slipc PRIVATE doctest trompeloeil::trompeloeil)
target_link_libraries(test_d_slipc PRIVATE callsit)
add_test(test_d_slipc test_d_slipc)
