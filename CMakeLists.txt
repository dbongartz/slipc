cmake_minimum_required(VERSION 3.20)
project("SLIPC" VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)
enable_testing()

add_library(slipc
    src/slipc.c
)

# add_library(examplelib example_lib.c)
target_sources(slipc PUBLIC include/slipc.h)
# target_sources(slipc PRIVATE src/submodule.c)
target_include_directories(slipc
	INTERFACE include
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
	# PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/submodule
)
target_compile_features(slipc PRIVATE c_std_17)
target_compile_options(slipc PRIVATE -Wall -Wextra)

add_subdirectory(test)
