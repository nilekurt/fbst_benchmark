project(fbst VERSION 0.1 LANGUAGES CXX)

file(GLOB_RECURSE INC "include/*.hh")
file(GLOB_RECURSE SRC "src/*.cc")

add_library(fbst)
target_sources(fbst PRIVATE ${INC} ${SRC})
target_include_directories(fbst PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_link_libraries(fbst PRIVATE fbst_benchmark_copts_common)

configure_lto(fbst)
configure_tidy(fbst)

