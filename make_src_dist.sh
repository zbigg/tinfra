#!/bin/sh

set -e
base_files=$(mtn ls known)

bakefile_gen

autoconf_files="Makefile.in autoconf_inc.m4 configure"
msvs_files="*.sln *.vcproj makefile.vc"

all_files="${base_files} ${autoconf_files} ${msvs_files}"
ls -1d ${all_files}

VERSION=0.0.1-dev
rm -rf tinfra-dev-${VERSION}.zip
zip tinfra-dev-${VERSION}.zip ${all_files}
