#!/bin/sh

set -e
base_files=$(mtn ls known)
generated_files=tinfra/config.h.in

bakefile_gen

tinfra_support="tinfra-support/spawn-test-common.sh tinfra-support/autoconf/* tinfra-support/common.bkl"
autoconf_files="Makefile.in autoconf_inc.m4 configure "
msvs_files="*.sln *.vcproj makefile.vc"
all_files="${base_files} ${generated_files} ${autoconf_files} ${msvs_files} ${tinfra_support}"
ls -1d ${all_files}

VERSION=0.0.1-dev
rm -rf tinfra-dev-${VERSION}.zip
zip tinfra-dev-${VERSION}.zip ${all_files}
