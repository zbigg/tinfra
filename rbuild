#!/bin/bash
#
# rbuild script
#
#   Build source in remote location. Remote location can be on this
#   or remote system. 
#     - remote systems are accessed by ssh
#     - source files are transferred via rsync (must live on this and build host machine)
#
#   = Usage = 
#
#   Maintenance:
#
#   [envsettings] rbuild --init
#
#     Initialize build folder BUILD_HOST. Create source folder
#     if sources are remote.
#
#   [envsettings] rbuild --remove
#
#     Remove build folder on BUILD_HOST and source 
#     if it's remote.
#
#   rbuild <make options>
#
#       Builds this source ($PWD) in folder ../build/${platform}/${name}
#       where
#           platform - is name of platform (i686, x86_64 ...)
#           name = $(basename $(PWD))
#       It invokes configure script and then invoke MAKE
#       with <make options>
#
#
#    rbuild ...
#
#         Specifies that build must be done on BUILD_HOST (ex. user@host.com)
#         System is accessed via ssh, scp & rsync.
#
#    BUILD_CONF=<filename> rbuild ...
#
#         Read the <filename> for additional options.
#
#  = Options = 
#
#    These options can be specified in environment or via configuration files:
#
#    BUILD_HOST=local 
#        build on local machine
#
#    BUILD_HOST=<anything>
#        build on remote machine <anything>
#        .rbuild-${BUILD_HOST} file is sourced for additional options.
#        implies default remote_src=/tmp/rbuild/$(name)
#
#    BUILD_HOST_CONFIGURE_OPTIONS="<anything>"
#        additional options for configure script
#
#    MAKE=<make command>
#        make command invoked to run the build
#
#  = Issues = 
#  
#  1. $(platform) is "uname -i" which sometimes yields unknown :/
#  2. This tool should be really called "rmake".
#
#  Author: Zbigniew Zagorski <z.zagorski@gmail.com>
#
set -e
#set -x

rbuild_root=$(dirname $(readlink -f $0))

name=${name-$(basename $PWD)}

BUILD_HOST=${BUILD_HOST-local}

sync()
{
        echo "LOCAL:  rsync $1 -> $2" 1>&2
        local RSYNC_OPTS="--exclude-from=${rbuild_root}/rbuild-rsync-exclude.txt -r -z -i --copy-links --times"
	if [ -f .rbuild-exclude.txt ] ; then
		RSYNC_OPTS="$RSYNC_OPTS --exclude-from=.rbuild-exclude.txt"
	fi
        rsync $RSYNC_OPTS $1 $2
}

MAKE=${MAKE-make}

custom_config="${BUILD_CONF}"
if [ -n "${BUILD_CONF}" ] ; then 
    echo "$0: loading config'${custom_config}'"
    source ${custom_config}
fi

if [ ${BUILD_HOST} = local ] ; then
    invoke_immediate() {
            "$@"
    }

    invoke() {
            "$@"
    }

    remote_path() {
            echo $1
    }

    copy() {
            cp "$@"
    }
else
    invoke_immediate() {
            echo "REMOTE: $@" 1>&2
            ssh $BUILD_HOST "$@"
    }
    invoke() {
            echo "REMOTE: $BUILD_HOST: $@" 1>&2
            ssh $BUILD_HOST "$@"
    }
    remote_path(){
            echo $BUILD_HOST:$1
    }

    copy()
    {
            echo "LOCAL:  scp $1 -> $2" 1>&2
            scp $1 $2
    }
fi
build_host_config=.rbuild-${BUILD_HOST}
if [ -f ${build_host_config} ] ; then
    echo "$0: loading host config '${build_host_config}'"
    source ${build_host_config}
fi

if [ ${BUILD_HOST} != local ] ; then
    remote_src=${remote_src-/tmp/rbuild/src}
    base_build_dir=${base_build_dir-/tmp/rbuild/build}
fi

if [ -z "$base_build_dir" ] ; then
	platform="$(invoke_immediate uname -s)-$(invoke_immediate uname -m)"
fi

base_build_dir=${base_build_dir-../build/$platform}
	
build_dir=${base_build_dir}/${name}

base_srcdir=$(pwd)

created_folders="${build_dir}"

echo "BUILD_HOST = $BUILD_HOST"
echo "SRC  = $(pwd)"
echo "BUILD = ${build_dir}"


if [ -n "${remote_src}" ] ; then
    build_scrdir=${remote_src}/${name}	
    created_folders="${created_folders} ${build_scrdir}"
else
    build_scrdir=${base_srcdir}
fi

REMOTESCRIPT=.rbuild-script

init_build_area() {
    invoke_immediate mkdir -p ${created_folders}

    REMOTESCRIPT_TMP=$(mktemp)
    cat > $REMOTESCRIPT_TMP <<EOF
cd ${build_dir}
if [ ! -f Makefile ] ; then
	if [ ! -f ./config.status ] ; then
		echo "[in \$(pwd)]" ${build_scrdir}/configure $BUILD_HOST_CONFIGURE_OPTIONS "$@"
		${build_scrdir}/configure $BUILD_HOST_CONFIGURE_OPTIONS "$@"
	fi
	if [ ! -f Makefile ] ; then
		echo "[in \$(pwd)]"  ./config.status
		./config.status
	fi
fi
exec make "\$@"

EOF
    copy $REMOTESCRIPT_TMP $(remote_path ${build_dir}/$REMOTESCRIPT)
    rm -rf $REMOTESCRIPT_TMP
}

remove_build_area() {
    invoke_immediate rm -rf ${created_folders}
}

if [ "$1" = "--init" ] ; then
    shift
    echo "$0: inializing build. configure options: $@"
        
    init_build_area "$@"
    exit
fi

if [ "$1" = "--remove" ] ; then
    echo "$0: removing build area"
    remove_build_area
    exit
fi

#
# sync & make
#

if [ -n "${remote_src}" ] ; then
	sync ${base_srcdir}/ $(remote_path ${remote_src}/$name/)
fi

invoke_immediate /bin/sh ${build_dir}/$REMOTESCRIPT "$@"

