#!/bin/sh

set -e

prepare() {
    if [ -f Bakefiles.bkgen ]
    then
        bakefile_gen
    fi
    LOCAL_ACDIR="$(dirname $(which bakefile))/../share/aclocal"
    
    [ -d ${LOCAL_ACDIR} ] && ACLOCAL_FLAGS="-I ${LOCAL_ACDIR}"
    
    aclocal ${ACLOCAL_FLAGS}

    if [ -n "$AUTOCONF_CONFIG_FILE" ]
    then
        autoheader
    fi
    autoconf
}
distclean() {
    make distclean || true
    bakefile_gen --clean
    rm -rf .bakefile_gen.state autom4te.cache autoconf_inc.m4 test_result aclocal.m4
    find . -name "*.d" | xargs rm -rf
    find . -name "*.o" | xargs rm -rf
    echo "still unknown files"
    mtn ls unknown
}


coverage() {
    covname=coverage.lcov
    set -e -x
    lcov --directory . --zerocounters
    ./unittests
    lcov --directory . --capture --output-file ${covname} -b $(pwd)
    lcov --extract ${covname} "$(pwd)/**" -o ${covname}
    rm -rf covresults
    genhtml -p $(pwd) -o covresults ${covname}
}

usage() {
    cat <<EOF
usage: build-helper command options

commands:
    prepare       prepare build system (invoke bakefile, autotools if needed)
    distclean     clean as much as possible (bakefile, autoconf, aclocal, .... output)
    
    conf-devel    configure for devel (-g -O0 -Wall -Wextra ...)
    conf-coverage configure for coverage tests (devel + -fprofile-arcs -ftest-coverage)
    
    coverage      run unittests and generate coverage report using lcov
    help          help on this tool
EOF
}

action="$1"
shift
case "${action}" in
    --help|help)
        usage
        ;;
    prepare)
        prepare
        ;;
    distclean)
        distclean
        ;;
        
    conf-devel)
        CXXFLAGS="-g -O0 -Wall -Wextra" ./configure "$@"
        ;;
    conf-coverage)
        CXXFLAGS="-fprofile-arcs -ftest-coverage -O0 -g -Wall -Wextra" LDFLAGS="-fprofile-arcs -ftest-coverage" ./configure "$@"
        ;;
        
    coverage)
        coverage
        ;;
    
    *)
        usage >&2
        exit 1
        ;;
esac
