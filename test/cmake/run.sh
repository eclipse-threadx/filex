#!/bin/bash

cd $(dirname $0)

# FileX has no test harness of its own. This clone supplies the runner, the
# CMake toolchain file and the ThreadX library the tests link against, so which
# ref it sits on is part of the build definition rather than a convenience.
threadx_ref=v6.4.1_cert
threadx_url=https://github.com/eclipse-threadx/threadx.git

if [ -d threadx ]; then
    # Compared against threadx's own path because git searches upwards: asked
    # inside a directory that is not a clone, it answers for the repository this
    # tree sits in, and the harness would then be taken for pinned.
    if [ "$(git -C threadx rev-parse --show-toplevel 2>/dev/null)" != "$(cd threadx && pwd -P)" ]; then
        echo "$(pwd)/threadx is not a git clone. Remove it and run again." >&2
        exit 1
    fi

    # The branch name is what is compared, not the commit: the pin is to the
    # branch, which advances, and a commit comparison would pin to a point on it.
    checked_out=$(git -C threadx rev-parse --abbrev-ref HEAD 2>/dev/null)
    if [ "$checked_out" != "$threadx_ref" ]; then
        echo "Test harness clone is on '$checked_out', not $threadx_ref. Re-fetching."
        git -C threadx fetch --depth 1 origin $threadx_ref || exit 1
        git -C threadx checkout -B $threadx_ref FETCH_HEAD || exit 1
    fi
else
    git clone $threadx_url --depth 1 --branch $threadx_ref threadx || exit 1
fi

echo "Test harness: threadx $threadx_ref at $(git -C threadx rev-parse --short HEAD)"

# Recreated on every run, so a link left pointing elsewhere cannot be reused.
ln -sfn threadx/scripts/cmake_bootstrap.sh .run.sh

./.run.sh $*
