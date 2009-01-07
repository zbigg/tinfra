#!/bin/sh

. $(basename $0)/tinfra-common/spawn-test-common.sh

generic_test unittests

exit $failed