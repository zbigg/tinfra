#!/bin/sh

dist="out/$1"
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
        echo -e "${test_name}\t\t (success)"
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
        echo -e "${test_name}\t\t (FAIL)"
        if [ -n "$*" ] ; then
            echo "${test_name}: $*"
        fi
    ) | tee -a ${test_log}
    failed=1
}

test_segv()
{
    test_name="test_fatal_exception-segv"
    test_log="${test_log_dir}/${test_name}.log"
    if ! $dist/test_fatal_exception segv &> ${test_log} ; then
        if grep -q "Fatal exception occurred" ${test_log} ; then
            success ${test_name}
        else
            fail ${test_name} "should report exception out stdout/stderr"
        fi        
    else
        fail ${test_name} "should exit with non-zero exitcode"
    fi
}
generic_test()
{
    test_name="$1"
    test_log="${test_log_dir}/${test_name}.log"
    if $dist/${test_name} &> ${test_log} ; then
        success ${test_name}
    else
        fail ${test_name}
    fi
}

test_segv
generic_test unittests

exit $failed