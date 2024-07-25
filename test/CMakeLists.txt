find_package(Catch2 3)
if(NOT Catch2_FOUND)
    message(NOTICE "Could not find Catch2, fetching from git")
    Include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.4.0
    )

    FetchContent_MakeAvailable(Catch2)
endif()

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

add_executable(unittests test_slipc.cc)
target_link_libraries(unittests PRIVATE Catch2::Catch2WithMain)
target_link_libraries(unittests PRIVATE slipc)
target_compile_features(unittests PRIVATE cxx_std_23)
catch_discover_tests(unittests)
