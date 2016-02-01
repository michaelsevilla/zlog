PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS zlog.proto)
add_library(zlog_proto SHARED ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(zlog_proto
    ${PROTOBUF_LIBRARIES}
)

####

add_library(zlog_seqr SHARED
    libseqr.cc
)

target_link_libraries(zlog_seqr
    zlog_proto
)

####

add_library(libzlog SHARED
    libzlog.cc
    stream.cc
    aio.cc
    stripe_history.cc
    log_mapper.cc
)

target_include_directories(libzlog
    PUBLIC ${LIBRADOS_INCLUDE_DIRS}
    PUBLIC ${LIBCLS_ZLOG_CLIENT_INCLUDE_DIRS}
)

target_link_libraries(libzlog
    zlog_proto
    ${LIBRADOS_LIBRARIES}
    ${LIBCLS_ZLOG_CLIENT_LIBRARIES}
    ${Boost_LOG_LIBRARY}
)

set_target_properties(libzlog PROPERTIES COMPILE_DEFINITIONS "BOOST_LOG_DYN_LINK")
set_target_properties(libzlog PROPERTIES OUTPUT_NAME "zlog")

####

add_library(zstate SHARED
    zstate/skytype.cc
)

target_link_libraries(zstate
    libzlog
)

####

add_executable(zlog-seqr
    seqr-server.cc
)

target_link_libraries(zlog-seqr
    libzlog
    zlog_proto
    zlog_seqr
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
)

####

add_executable(append-check append-check.cc)
target_link_libraries(append-check
    libzlog
    zlog_seqr
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
)

####

add_executable(zlogtool zlog-tool.cc)
set_target_properties(zlogtool PROPERTIES OUTPUT_NAME "zlog")
target_link_libraries(zlogtool
    libzlog
    zlog_seqr
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
)

####

add_executable(zlog-seqr-bench
    zlog-seqr-bench.cc
)

target_link_libraries(zlog-seqr-bench
    libzlog
    zlog_seqr
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
)

####

add_executable(zlog-bench-ng
    zlog-bench-ng.cc
)

target_link_libraries(zlog-bench-ng
    libzlog
    zlog_seqr
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    atomic
)

####

add_executable(stream-stress
    stream-stress.cc
)

target_link_libraries(stream-stress
    libzlog
    zlog_seqr
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
)

####

add_library(gtest
    gtest/src/gtest-all.cc
    gtest/src/gtest_main.cc
)

target_include_directories(gtest
    PUBLIC gtest/include
    PRIVATE gtest
)

####

add_executable(zlog-test
    test.cc
)

target_link_libraries(zlog-test
    libzlog
    gtest
    zlog_seqr
    ${Boost_SYSTEM_LIBRARY}
)

####

add_executable(zstate-test
    zstate/test.cc
)

target_link_libraries(zstate-test
    libzlog
    gtest
    zstate
    zlog_seqr
    ${Boost_SYSTEM_LIBRARY}
)