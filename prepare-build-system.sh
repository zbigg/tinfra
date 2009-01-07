#!/bin/sh


# bootstrap-project
#
# prepare project for build
#
# invokes
#    bakefile
#    aclocal
#    autoconf
#
# checks if tinfra-common is installed
#
# TODO: it should be named tinfra-project
# and contain few commands
#   prepare - do what is in here
#   clean   - clean all
# actual implementation should go be in tinfra-common
#
# TODO tinfra-common should be renamed to tinfra-support
set -e
PNAME=$(basename $0)
if ! [ -d tinfra-common ] ; then
    echo "$PNAME: tinfra-common is missing, trying install one" >&2
    if ! mtn -b pl.reddix.tinfra-common co tinfra-common ; then
        echo "$PNAME: tinfra-common is hard requirement, get tinfra monotone database or" >&2
        echo "$PNAME: place tinfra-common in folder ${PWD}tinfra-common" >&2
        exit 1
    fi
fi

bakefile_gen
aclocal
#autoheader
autoconf
