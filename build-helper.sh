#!/bin/sh

set -e

prepare() {
    if [ -f Bakefiles.bkgen ]
    then
        bakefile_gen
    fi

    aclocal

    if [ -n "$AUTOCONF_CONFIG_FILE" ]
    then
        autoheader
    fi
    autoconf
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
    *)
        usage >&2
        exit 1
        ;;
esac
