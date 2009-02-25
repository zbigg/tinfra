#!/bin/bash

#set -e -x

rbuild_root=$(readlink -f $(dirname $0))

name=${name-$(basename $PWD)}

BUILD_HOST=${BUILD_HOST-local}

sync()
{
        echo "LOCAL:  rsync $1 -> $2" 1>&2
        local RSYNC_OPTS="--exclude-from=${rbuild_root}/rbuild-rsync-exclude.txt -r -z -v --copy-links --times"
        rsync $RSYNC_OPTS $1 $2
}

MAKE=${MAKE-make}

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
    remote_src=/tmp/rbuild/src
    base_build_dir=/tmp/rbuild/build
fi

build_host_config=.rbuild-${BUILD_HOST}
if [ -f ${build_host_config} ] ; then
    echo "$0: loading host config '${build_host_config}'"
    source ${build_host_config}
fi

echo "REMOTE = $BUILD_HOST"
echo "LOCAL  = local"

if [ -z "$base_build_dir" ] ; then
	platform=$(invoke_immediate uname -p)
fi

base_build_dir=${base_build_dir-../build/$platform}
	
build_dir=${base_build_dir}/${name}

base_srcdir=$(pwd)

created_folders="${build_dir}"

if [ -n "${remote_src}" ] ; then
    build_scrdir=${remote_src}/${name}	
    created_folders="${created_folders} ${build_scrdir}"
else
    build_scrdir=${base_srcdir}
fi

PLAMAKEFILE=Makefile.plamake

init_build_area() {
    invoke_immediate mkdir -p ${created_folders}
	
    PLAMAKEFILE_TMP=$(mktemp)
    cat > $PLAMAKEFILE_TMP <<EOF
-include Makefile

Makefile: ./config.status
	./config.status
./config.status: ${build_scrdir}/configure
	${build_scrdir}/configure $BUILD_HOST_CONFIGURE_OPTIONS
EOF
    copy $PLAMAKEFILE_TMP $(remote_path ${build_dir}/$PLAMAKEFILE)
    rm -rf $PLAMAKEFILE_TMP
}

remove_build_area() {
    invoke_immediate rm -rf ${created_folders}
}

if [ "$1" = "--init" ] ; then
	echo "$0: inializing build"
	init_build_area
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

invoke_immediate ${MAKE} -C ${build_dir} -f $PLAMAKEFILE "$@"
