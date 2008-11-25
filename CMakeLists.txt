cmake_minimum_required (VERSION 2.6)

project (tinfra)

SET(tinfra_basic_sources 
    tinfra/cmd.cpp
    tinfra/exception.cpp
    tinfra/exeinfo.cpp
    tinfra/fmt.cpp
    tinfra/fs.cpp
    tinfra/path.cpp
    tinfra/regexp.cpp
    tinfra/runner.cpp
    tinfra/runtime.cpp
    tinfra/server.cpp
    tinfra/string.cpp
    tinfra/subprocess.cpp
    tinfra/symbol.cpp
    tinfra/threadcmn.cpp
    tinfra/tinfra.cpp
    tinfra/tstring.cpp

    tinfra/io/socket.cpp
    tinfra/io/stream.cpp
    tinfra/os_common.cpp
    )
    
SET(tinfra_posix_sources
    tinfra/posix/posix_io.cpp
    tinfra/posix/posix_runtime.cpp
    tinfra/posix/posix_subprocess.cpp
    tinfra/posix/thread.cpp
    )

SET(tinfra_xml_sources
    tinfra/xml/XMLStream.cpp
    tinfra/xml/XMLStreamReader.cpp
    )

SET(tinfra_test_sources
    tinfra/test.cpp
    tinfra/test_exeinfo.cpp
    #tinfra/test_fatal_exception.cpp
    tinfra/test_fmt.cpp
    tinfra/test_fs.cpp
    tinfra/test_main.cpp
    tinfra/test_multitype_map.cpp
    tinfra/test_path.cpp
    tinfra/test_queue.cpp
    tinfra/test_regexp.cpp
    tinfra/test_server.cpp
    tinfra/test_string.cpp
    tinfra/test_subprocess.cpp
    tinfra/test_symbol.cpp
    tinfra/test_test.cpp
    tinfra/test_thread.cpp
    tinfra/test_tstring.cpp
    tinfra/io/test_stream.cpp
    )


set( CMAKE_MODULE_PATH .)

include(CheckIncludeFiles)
include(CheckFunctionExists)

check_include_files(io.h HAVE_IO_H)
check_function_exists(opendir HAVE_OPENDIR)
check_function_exists(nanosleep HAVE_NANOSLEEP)

find_package(Threads REQUIRED)
find_package(Pcre REQUIRED)

configure_file(tinfra/config.h.in tinfra/config.h)
include_directories(
    ${TINFRA_SOURCE_DIR} 
    ${PCRE_INCLUDE_DIR}
    ) 
add_library (tinfra ${tinfra_basic_sources} ${tinfra_posix_sources}) 
add_library (tinfra_xml ${tinfra_xml_sources}) 

add_executable (unittests ${tinfra_test_sources}) 
add_test(tinfra_test unittests)

# TODO
#       search for pcre
#       search for unittest++
#       search for expat
#       search for pthread
#include(FindPkgConfig)
#pkg_check_modules(PCRE pcre)

find_path (PTHREAD_H pthread.h)

target_link_libraries(unittests unittest++)
target_link_libraries(tinfra ${PCRE_LIBRARY})
target_link_libraries(tinfra pthread)
target_link_libraries(unittests tinfra)

