#!/bin/bash
# The bootstrap takes its serial configuration loop when CTEST_PARALLEL_LEVEL is
# set, and passes the value on to ctest. 4 matches NetX Duo, USBX and GUIX.
CTEST_PARALLEL_LEVEL=4 $(dirname `realpath $0`)/../test/cmake/run.sh test all
