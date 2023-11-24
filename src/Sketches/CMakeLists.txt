include("${ClickHouse_SOURCE_DIR}/cmake/dbms_glob_sources.cmake")
add_headers_and_sources(clickhouse_sketches .)

#list(REMOVE_ITEM clickhouse_sketches_sources registerSketches.cpp)
#list(REMOVE_ITEM clickhouse_sketches_sources SketchFactory.cpp)
#list(REMOVE_ITEM clickhouse_sketches_headers registerSketches.h)
#list(REMOVE_ITEM clickhouse_sketches_headers SketchFactory.h)

extract_into_parent_list(clickhouse_sketches_sources dbms_sources
    ISketch.cpp
    SketchFactory.cpp
)

extract_into_parent_list(clickhouse_sketches_headers dbms_headers
    ISketch.h
    SketchFactory.h
)

add_library(clickhouse_sketches ${clickhouse_sketches_headers} ${clickhouse_sketches_sources})
target_link_libraries(clickhouse_sketches PRIVATE dbms PUBLIC ch_contrib::xxHash)
