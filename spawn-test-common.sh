#!/bin/sh

dist="$1"
if [ ! -d "$dist" ] ; then
    echo "$0: output dir '$dist' doesn't exist, build the project" 1>&2  
    exit 2
fi
test_log_dir=${dist}/test_result
mkdir -p ${test_log_dir}

interactive=no

failed=0

success() {
    test_name="$1"
    test_log="${test_log_dir}/${test_name}.log"
    echo "-----------------" >> $test_log
    shift
    (
        echo "${test_name}\t\t (success)"
        if [ -n "$*" ] ; then
            echo "${test_name}: $*"
        fi
    ) | tee -a ${test_log}
}
fail()
{
    test_name="$1"
    test_log="${test_log_dir}/${test_name}.log"
    echo "-----------------" >> $test_log
    shift
    (
        echo "${test_name}\t\t (FAIL)"
        if [ -n "$*" ] ; then
            echo "${test_name}: $*"
        fi
    ) | tee -a ${test_log}
    failed=1
}

generic_test()
{
    test_name="$1"
    test_log="${test_log_dir}/${test_name}.log"
    if $dist/${test_name} > ${test_log} 2>&1 ; then
        success ${test_name}
    else
        fail ${test_name}
    fi
}
