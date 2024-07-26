cmake_minimum_required(VERSION 3.20)
project("SLIPC"
	VERSION 0.1.0
	DESCRIPTION "Simple Lightweight Internet Protocol in C"
	LANGUAGES C CXX
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

add_library(slipc_io src/slipc_io.c)
target_include_directories(slipc_io
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(slipc_io PRIVATE c_std_17)
target_compile_options(slipc_io PRIVATE -Wall -Wextra)

add_library(slipc src/slipc.c)
target_link_libraries(slipc PRIVATE slipc_io)
target_include_directories(slipc
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(slipc PRIVATE c_std_17)
target_compile_options(slipc PRIVATE -Wall -Wextra)

option(${PROJECT_NAME}_ENABLE_TESTS "Enable unit tests for SLPIC." OFF)
if(${PROJECT_NAME}_ENABLE_TESTS)
add_subdirectory(test)
endif()
