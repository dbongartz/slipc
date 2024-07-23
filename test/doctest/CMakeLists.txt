add_library(doctest INTERFACE)
target_include_directories(doctest
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_features(doctest INTERFACE cxx_std_17)
