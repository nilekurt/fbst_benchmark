project(fbst_benchmark_main VERSION 0.1 LANGUAGES CXX)

file(GLOB_RECURSE INC "include/*.hh")
file(GLOB_RECURSE SRC "src/*.cc")

add_executable(fbst_benchmark_main)
target_sources(fbst_benchmark_main PRIVATE ${INC} ${SRC})
target_link_libraries(fbst_benchmark_main PRIVATE fbst_benchmark_copts_common fbst)

configure_lto(fbst_benchmark_main)
configure_tidy(fbst_benchmark_main)

