#!/usr/bin/env bash

#set -x

test_dir=$(pwd)

. $(dirname $0)/tinfra-support/spawn-test-common.sh $(pwd)

test_segv()
{
    local test_name="test_fatal_exception-segv"
    local test_log_file=$(get_test_log_file $test_name)
    report_test_start ${test_name} ./test_fatal_exception segv
    ( 
        cd ${test_dir}
        ${RUNNER} ./test_fatal_exception segv 
    ) >> ${test_log_file} 2>&1
    test_exit_code=$?
    if [ ${test_exit_code} != "0" ] ; then
        if grep -q "fatal signal 11 received" ${test_log_file} ; then
            report_result ${test_name} ${test_exit_code} success
        else
            report_result ${test_name} ${test_exit_code} "FAIL, should report exception out stdout/stderr"
            failed=1
        fi        
    else
        report_result ${test_name} ${test_exit_code} "FAIL, should exit with non-zero exitcode"
        failed=1
    fi
}

topsrc_dir=$1
testresources_dir=${1}/tests/resources

test_segv
generic_test plain_unittests ${RUNNER} ./unittests --srcdir=${1} #-D ${testresources_dir}

generic_test_summary

exit $failed

