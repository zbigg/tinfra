#!/bin/sh

. $(dirname $0)/tinfra-support/spawn-test-common.sh

generic_test unittests

exit $failed