#!/bin/bash
##############################################################################
# Copyright (c) 2024 Microsoft Corporation
# Copyright (c) 2026 Eclipse ThreadX contributors
#
# This program and the accompanying materials are made available under the
# terms of the MIT License which is available at
# https://opensource.org/licenses/MIT.
#
# SPDX-License-Identifier: MIT
##############################################################################

set -euo pipefail

cd "$(dirname "$0")"
revision=$(cat threadx-revision.txt)

if [ ! -e threadx ]; then
    git init -q threadx
    git -C threadx remote add origin https://github.com/eclipse-threadx/threadx.git
    timeout 180 git -C threadx fetch --depth 1 origin "$revision"
    git -C threadx checkout -q --detach FETCH_HEAD
fi

actual_revision=$(git -C threadx rev-parse HEAD)
if [ "$actual_revision" != "$revision" ]; then
    echo "ThreadX checkout does not match threadx-revision.txt." >&2
    exit 1
fi

bootstrap=threadx/scripts/cmake_bootstrap.sh
if [ ! -f "$bootstrap" ]; then
    echo "ThreadX bootstrap script is missing." >&2
    exit 1
fi

if [ "${1:-}" = test ] && [ "${2:-}" = all ] && [ "${TX_COVERAGE:-OFF}" = ON ]; then
    ./coverage.sh --clean
fi

ln -sfn "$bootstrap" .run.sh
exec ./.run.sh "$@"
