#!/bin/sh

if [ ! -d "$test_dir" ] ; then
    echo "$0: output dir '$test_dir' doesn't exist, build the project" 1>&2  
    exit 2
fi

test_log_file_dir=${test_dir}/test_result
mkdir -p ${test_log_file_dir}

interactive=no

failed=0

get_test_log_file()
{
    echo ${test_log_file_dir}/${1}.log
}

report_test_start()
{
    test_name="$1"
    shift
    test_command="$*"
    local test_log_file=$(get_test_log_file $test_name)
    (   
        cd $test_dir
        set -e
        echo "TEST EXECUTION LOG"
        echo "time:             $(date)"
        echo "-------------------------"
        echo "test name:        ${test_name}"
        echo "test command:     ${test_command}"
        echo "working directory $(pwd)"
        echo "test environment follows"
        env
        echo "-------------------------"
        echo "system:           $(uname -a)"
        echo "------------------------- (test output begins on next line)"
    ) > ${test_log_file}
}

report_result()
{
    
    local test_name="$1"
    local test_exit_code="$2"
    msg="$3"
    local test_log_file=$(get_test_log_file $test_name)
    (
        echo "------------------------- (test output end)"
        echo "test exit code:   ${test_exit_code}"
        echo "overall result:   ${msg}"
    ) >> ${test_log_file}
    printf "%40s (${msg})\n" ${test_name}
}

generic_test()
{
    local test_name="$1"
    shift
    test_command="$*"
    local test_log_file=$(get_test_log_file $test_name)
    report_test_start ${test_name} ${test_command}
    ( (
        cd ${test_dir}
        ${test_command}
    )  2>&1 ) >> ${test_log_file} 
    local test_exit_code=$?
    if [ "$test_exit_code" = "0" ]; then
        msg="success"
    else
        msg="FAIL"
        failed=1
    fi
    report_result $test_name $test_exit_code $msg
}

generic_test_summary()
{
	echo "overall result: ${msg}($failed)"
	echo "logs folder:    ${test_log_file_dir}"
}

