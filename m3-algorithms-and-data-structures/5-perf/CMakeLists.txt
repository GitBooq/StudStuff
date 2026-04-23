add_executable(BenchCompare bench_compare.cc)
target_link_libraries(BenchCompare PRIVATE 
    benchmark::benchmark
)
target_compile_options(BenchCompare PRIVATE -O2)
target_compile_features(BenchCompare PRIVATE cxx_std_23)