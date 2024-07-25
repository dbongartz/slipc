cmake_minimum_required(VERSION 3.20)
project("SLIPC" VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)
enable_testing()

add_library(slipc_io src/slipc_io.c)
target_include_directories(slipc_io
	INTERFACE include
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(slipc_io PRIVATE c_std_17)
target_compile_options(slipc_io PRIVATE -Wall -Wextra)

add_library(slipc src/slipc.c)
target_link_libraries(slipc PRIVATE slipc_io)
target_include_directories(slipc
	INTERFACE include
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(slipc PRIVATE c_std_17)
target_compile_options(slipc PRIVATE -Wall -Wextra)

add_subdirectory(test)
