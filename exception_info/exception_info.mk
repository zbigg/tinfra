#
# exception_info module
#

## dynamic linking test

exception_info_gcc_dynamic_test_DIR=$(exception_info_DIR)
exception_info_gcc_dynamic_test_SOURCES=\
	exception_info_test.cpp \
	exception_info_gcc.cpp \
	exception_info_gcc_dynamic.cpp

exception_info_gcc_dynamic_test_EXT_LIBS=tinfra-test tinfra
PROGRAMS += exception_info_gcc_dynamic_test

TEST_PROGRAMS += exception_info_gcc_dynamic_test

## static linking test

exception_info_gcc_static_test_DIR=$(exception_info_DIR)
exception_info_gcc_static_test_SOURCES=\
	exception_info_test.cpp \
	exception_info_gcc.cpp \
	exception_info_gcc_static.cpp

exception_info_gcc_static_test_EXT_LIBS = tinfra-test tinfra
PROGRAMS += exception_info_gcc_static_test

exception_info_gcc_static_test_LDFLAGS += libstdc++-tinfra-throw.a

libstdc++-tinfra-throw.a: $(exception_info_DIR)/make_libstdc++-tinfra-throw.sh
	$(exception_info_DIR)/make_libstdc++-tinfra-throw.sh


TEST_PROGRAMS += exception_info_gcc_static_test


# we host call ranger for a while
call_ranger_test_DIR ?= $(exception_info_DIR)
call_ranger_test_DIR=$(exception_info_DIR)


call_ranger_test_DIR=$(exception_info_DIR)
call_ranger_test_SOURCES=\
	call_ranger.cpp \
	call_ranger_test.cpp \
	exception_info_gcc.cpp \
	exception_info_gcc_dynamic.cpp \
	safe_debug_print.cpp \
	safe_debug_print_test.cpp

call_ranger_test_EXT_LIBS=tinfra-test tinfra
PROGRAMS += call_ranger_test

TEST_PROGRAMS += call_ranger_test


# jedit: :tabSize=8:mode=makefile:

