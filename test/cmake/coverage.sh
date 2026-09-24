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
repo_root=$(cd ../.. && pwd)
report_dir=coverage_report
configurations=(
    default_build_coverage no_cache_build no_cache_standalone_build
    fault_tolerant_build_coverage no_check_build no_cache_fault_tolerant_build
    standalone_build_coverage standalone_fault_tolerant_build_coverage
    standalone_no_cache_fault_tolerant_build
)

check_report() {
    python3 - "$@" <<'PY'
import json
import pathlib
import sys
import xml.etree.ElementTree as ET

report = pathlib.Path(sys.argv[1])
root = ET.parse(report).getroot()
classes = root.findall('.//class')
if int(root.get('lines-valid', '0')) == 0 or not classes:
    raise SystemExit(f'{report}: report contains no measured FileX files')
if any(not item.get('filename', '').startswith('common/src/') for item in classes):
    raise SystemExit(f'{report}: report contains a source outside common/src')
if len(sys.argv) > 2:
    with open(sys.argv[2], encoding='utf-8') as stream:
        data = json.load(stream)
    if not data.get('files'):
        raise SystemExit(f'{sys.argv[2]}: tracefile contains no measured files')
    if any(not item.get('file', '').startswith('common/src/') for item in data['files']):
        raise SystemExit(f'{sys.argv[2]}: tracefile contains a source outside common/src')
print(f"{report}: lines {root.get('lines-covered')}/{root.get('lines-valid')} "
      f"({float(root.get('line-rate', '0')) * 100:.2f}%), branches "
      f"{root.get('branches-covered')}/{root.get('branches-valid')} "
      f"({float(root.get('branch-rate', '0')) * 100:.2f}%)")
PY
}

check_merged_floor() {
    python3 - "$1" <<'PY'
import sys
import xml.etree.ElementTree as ET

root = ET.parse(sys.argv[1]).getroot()
for label, attribute, minimum in (
    ('line', 'lines', 999),
    ('branch', 'branches', 994),
):
    covered = int(root.get(f'{attribute}-covered', '0'))
    valid = int(root.get(f'{attribute}-valid', '0'))
    if valid == 0 or covered * 1000 < valid * minimum:
        raise SystemExit(
            f'{sys.argv[1]}: {label} coverage {covered}/{valid} '
            f'is below {minimum / 10:.1f}%'
        )
PY
}

if [ "${1:-}" = --clean ]; then
    if [ -d "$report_dir" ]; then
        rm -r -- "$report_dir"
    fi
    if [ -d build ]; then
        find build -type f -name '*.gcda' -delete
    fi
    exit 0
fi

if [ "${1:-}" = --merge ]; then
    trace_args=()
    for configuration in "${configurations[@]}"; do
        base="$report_dir/per_configuration/$configuration"
        for path in "$base.json" "$base.xml" "$base/index.html"; do
            if [ ! -s "$path" ]; then
                echo "Missing or empty coverage report: $path" >&2
                exit 1
            fi
        done
        check_report "$base.xml" "$base.json"
        trace_args+=(--add-tracefile "$base.json")
    done

    mkdir -p "$report_dir/merged"
    gcovr -r "$repo_root" "${trace_args[@]}" --xml-pretty \
        --output "$report_dir/merged.xml"
    gcovr -r "$repo_root" "${trace_args[@]}" --html --html-details \
        --output "$report_dir/merged/index.html"
    check_report "$report_dir/merged.xml"
    check_merged_floor "$report_dir/merged.xml"
    exit 0
fi

configuration="${1:-}"
valid=0
for item in "${configurations[@]}"; do
    if [ "$configuration" = "$item" ]; then
        valid=1
        break
    fi
done
if [ "$valid" -ne 1 ]; then
    echo "Unknown coverage configuration: $configuration" >&2
    exit 1
fi

cc_name=$(basename "${CC:-gcc}")
if [ -n "${GCOV:-}" ]; then
    gcov="$GCOV"
elif [[ "$cc_name" = gcc* ]]; then
    gcov="gcov${cc_name#gcc}"
else
    gcov=gcov
fi
if ! command -v "$gcov" >/dev/null 2>&1; then
    echo "Coverage tool $gcov is unavailable." >&2
    exit 1
fi

objects="$PWD/build/$configuration/filex/CMakeFiles/filex.dir/common/src"
if [ ! -d "$objects" ] || [ -z "$(find "$objects" -name '*.gcda' -print -quit)" ]; then
    echo "No FileX coverage data for $configuration." >&2
    exit 1
fi

base="$report_dir/per_configuration/$configuration"
mkdir -p "$base"
# Regression tests use a separate driver with fault injection.
gcovr --gcov-executable "$gcov" -r "$repo_root" -f "$repo_root/common/src" \
    -e "$repo_root/common/src/fx_ram_driver.c" \
    "$objects" --json "$base.json" --xml-pretty --output "$base.xml"
gcovr --gcov-executable "$gcov" -r "$repo_root" -f "$repo_root/common/src" \
    -e "$repo_root/common/src/fx_ram_driver.c" \
    "$objects" --html --html-details --output "$base/index.html"
check_report "$base.xml" "$base.json"
