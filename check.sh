#!/bin/sh

. $(dirname $0)/tinfra-support/spawn-test-common.sh

test_segv()
{
    test_name="test_fatal_exception-segv"
    test_log="${test_log_dir}/${test_name}.log"
    if ! $dist/test_fatal_exception segv > ${test_log} 2>&1 ; then
        if grep -q "fatal signal 11 received" ${test_log} ; then
            success ${test_name}
        else
            fail ${test_name} "should report exception out stdout/stderr"
        fi        
    else
        fail ${test_name} "should exit with non-zero exitcode"
    fi
}

test_segv
generic_test unittests

exit $failed