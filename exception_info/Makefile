

#exception_info_DIR=.
include makefoo_configured_defs.mk

include exception_info.mk

MAKEFOO_dir=../../makefoo
MAKEFOO=$(MAKEFOO_dir)/main.mk

MAKEFOO_USE += native
MAKEFOO_USE += autolib
MAKEFOO_USE += test-program
MAKEFOO_USE += cppcheck

include $(MAKEFOO)
