#!/bin/sh

set -e

prepare() {
    if [ -f Bakefiles.bkgen ]
    then
        bakefile_gen
    fi

    aclocal -I "$(dirname $(which bakefile))/../share/aclocal"

    if [ -n "$AUTOCONF_CONFIG_FILE" ]
    then
        autoheader
    fi
    autoconf
}
distclean() {
    make distclean
    bakefile_gen --clean
    rm -rf .bakefile_gen.state autom4te.cache autoconf_inc.m4 test_result aclocal.m4
    @echo "still unknown files"
    mtn ls unknown
}

usage() {
    cat <<EOF
usage: build-helper command options

commands:
    prepare     prepare build system (invoke bakefile, autotools if needed)
    help        help on this tool
EOF
}
case "$1" in
    --help|help)
        usage
        ;;
    prepare)
        prepare
        ;;
    distclean)
        distclean
        ;;
    *)
        usage >&2
        exit 1
        ;;
esac
