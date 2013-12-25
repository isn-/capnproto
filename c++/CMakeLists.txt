cmake_minimum_required(VERSION 2.8)
project(capnproto CXX C)

option(USE_EXTERNAL_GTEST "Create external project for gtest, download and build it during `make`" ON)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
endif()

if ("${BUILD_SHARED_LIBS}" STREQUAL "")
    set(BUILD_SHARED_LIBS ON)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
list(APPEND CMAKE_CXX_FLAGS "-std=c++11")
link_libraries(stdc++)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 5)
execute_process(
    COMMAND git --git-dir ${CMAKE_CURRENT_SOURCE_DIR}/../.git rev-parse --short HEAD
    OUTPUT_VARIABLE GIT_REV
)
string(REPLACE "\n" "" GIT_REV ${GIT_REV})
set(VERSION_PATCH ${GIT_REV})
set(SO_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}-${VERSION_PATCH}")

find_package(Threads REQUIRED)
link_libraries(${CMAKE_THREAD_LIBS_INIT}) # -lpthread(s) for all targets

include(CTest)
enable_testing()

if (USE_EXTERNAL_GTEST)
    include(ExternalProject)
    ExternalProject_Add(gtest
        DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/gtest
        SVN_REPOSITORY http://googletest.googlecode.com/svn/tags/release-1.7.0
        UPDATE_COMMAND ""
        CMAKE_ARGS -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        INSTALL_COMMAND ""
    )

    ExternalProject_Get_Property(gtest source_dir)
    ExternalProject_Get_Property(gtest binary_dir)
    set(GTEST_LIBRARY "${binary_dir}/libgtest.a")
    set(GTEST_MAIN_LIBRARY "${binary_dir}/libgtest_main.a")
    set(GTEST_INCLUDE_DIRECTORIES ${source_dir}/include)
else()
    find_package(Gtest REQUIRED)
endif()

include_directories(${GTEST_INCLUDE_DIRECTORIES})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src)

set(kj_sources
    src/kj/common.c++
    src/kj/units.c++
    src/kj/memory.c++
    src/kj/refcount.c++
    src/kj/array.c++
    src/kj/string.c++
    src/kj/string-tree.c++
    src/kj/exception.c++
    src/kj/debug.c++
    src/kj/arena.c++
    src/kj/io.c++
    src/kj/mutex.c++
    src/kj/thread.c++
    src/kj/main.c++
    src/kj/parse/char.c++
)

add_library(kj ${kj_sources})
target_link_libraries(kj)
set_target_properties(kj PROPERTIES SOVERSION ${SO_VERSION})

set(kj_async_sources
    src/kj/async.c++
    src/kj/async-unix.c++
    src/kj/async-io.c++
)

add_library(kj-async ${kj_async_sources})
target_link_libraries(kj-async kj)
set_target_properties(kj-async PROPERTIES SOVERSION ${SO_VERSION})


set(capnp_sources
    src/capnp/c++.capnp.c++
    src/capnp/blob.c++
    src/capnp/arena.c++
    src/capnp/layout.c++
    src/capnp/list.c++
    src/capnp/any.c++
    src/capnp/message.c++
    src/capnp/schema.capnp.c++
    src/capnp/schema.c++
    src/capnp/schema-loader.c++
    src/capnp/dynamic.c++
    src/capnp/stringify.c++
    src/capnp/serialize.c++
    src/capnp/serialize-packed.c++
    src/capnp/json.c++
)

add_library(capnp ${capnp_sources})
target_link_libraries(capnp kj)
set_target_properties(capnp PROPERTIES SOVERSION ${SO_VERSION})


set(capnp_rpc_sources
    src/capnp/serialize-async.c++
    src/capnp/capability.c++
    src/capnp/dynamic-capability.c++
    src/capnp/rpc.c++
    src/capnp/rpc.capnp.c++
    src/capnp/rpc-twoparty.c++
    src/capnp/rpc-twoparty.capnp.c++
    src/capnp/ez-rpc.c++
)

add_library(capnp-rpc ${capnp_rpc_sources})
target_link_libraries(capnp-rpc capnp kj kj-async)
set_target_properties(capnp-rpc PROPERTIES SOVERSION ${SO_VERSION})

set(capnpc_sources
    src/capnp/compiler/md5.c++
    src/capnp/compiler/error-reporter.c++
    src/capnp/compiler/lexer.capnp.c++
    src/capnp/compiler/lexer.c++
    src/capnp/compiler/grammar.capnp.c++
    src/capnp/compiler/parser.c++
    src/capnp/compiler/node-translator.c++
    src/capnp/compiler/compiler.c++
    src/capnp/schema-parser.c++
)

add_library(capnpc ${capnpc_sources})
target_link_libraries(capnpc capnp kj)
set_target_properties(capnpc PROPERTIES SOVERSION ${SO_VERSION})

set(capnp_binary_sources
    src/capnp/compiler/module-loader.c++
    src/capnp/compiler/capnp.c++
)

add_executable(capnp-exe ${capnp_binary_sources})
set_target_properties(capnp-exe PROPERTIES OUTPUT_NAME capnp)
target_link_libraries(capnp-exe capnpc capnp kj)

add_executable(capnpc-capnp src/capnp/compiler/capnpc-capnp.c++)
target_link_libraries(capnpc-capnp capnp kj)

add_executable(capnpc-c++ src/capnp/compiler/capnpc-c++.c++)
target_link_libraries(capnpc-c++ capnp kj)


get_target_property(CAPNPC_EXECUTABLE capnp-exe LOCATION)
get_target_property(CAPNPC_CXX_EXECUTABLE capnpc-c++ LOCATION)
set(CAPNPC_IMPORT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src)

include(capnpc) # for capnp_generate_cpp()

set(test_capnpc_inputs
    src/capnp/test.capnp
    src/capnp/test-import.capnp
    src/capnp/test-import2.capnp
)

capnp_generate_cpp(test_capnpc_sources test_capnpc_headers ${test_capnpc_inputs})

set(capnp_test_sources
    ${test_capnpc_sources}
    src/kj/common-test.c++
    src/kj/memory-test.c++
    src/kj/refcount-test.c++
    src/kj/array-test.c++
    src/kj/string-test.c++
    src/kj/string-tree-test.c++
    src/kj/exception-test.c++
    src/kj/debug-test.c++
    src/kj/arena-test.c++
    src/kj/units-test.c++
    src/kj/tuple-test.c++
    src/kj/one-of-test.c++
    src/kj/function-test.c++
    src/kj/mutex-test.c++
    src/kj/async-test.c++
    src/kj/async-unix-test.c++
    src/kj/async-io-test.c++
    src/kj/parse/common-test.c++
    src/kj/parse/char-test.c++
    src/capnp/common-test.c++
    src/capnp/blob-test.c++
    src/capnp/endian-test.c++
    src/capnp/endian-fallback-test.c++
    src/capnp/endian-reverse-test.c++
    src/capnp/layout-test.c++
    src/capnp/any-test.c++
    src/capnp/message-test.c++
    src/capnp/capability-test.c++
    src/capnp/schema-test.c++
    src/capnp/schema-loader-test.c++
    src/capnp/dynamic-test.c++
    src/capnp/stringify-test.c++
    src/capnp/encoding-test.c++
    src/capnp/orphan-test.c++
    src/capnp/serialize-test.c++
    src/capnp/serialize-async-test.c++
    src/capnp/serialize-packed-test.c++
    src/capnp/rpc-test.c++
    src/capnp/rpc-twoparty-test.c++
    src/capnp/ez-rpc-test.c++
    src/capnp/test-util.c++
    src/capnp/compiler/lexer-test.c++
    src/capnp/compiler/md5-test.c++
    src/capnp/json-test.c++
)

install(TARGETS kj kj-async capnp capnp-rpc capnpc capnp-exe capnpc-capnp capnpc-c++
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

add_executable(capnp-test ${capnp_test_sources})
target_link_libraries(capnp-test ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY} capnpc capnp-rpc capnp kj-async kj)
add_test(capnp ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/capnp-test)
add_dependencies(capnp-test capnp-exe capnpc-c++)

add_executable(capnp-evolution-test src/capnp/compiler/evolution-test.c++)
target_link_libraries(capnp-evolution-test ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY} capnpc capnp-rpc capnp kj-async kj)
add_test(capnp-evolution ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/capnp-evolution-test)

if (USE_EXTERNAL_GTEST)
    add_dependencies(capnp-test gtest)
    add_dependencies(capnp-evolution-test gtest)
endif()
