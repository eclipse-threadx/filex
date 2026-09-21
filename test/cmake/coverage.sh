#!/bin/bash

set -e

cd $(dirname $0)

# Both -r and -f have to be absolute. A relative root resolved against whatever
# directory gcovr was started in is how a report containing zero files and an
# exit status of 0 is produced, which is the worst failure mode available here:
# a green run carrying an empty report. Rooting every configuration at the
# repository root rather than at common/src also makes the file names
# repo-relative, so the same source file is named identically in every
# configuration's report -- which is what lets --merge below union them at all,
# and what lets coverage_union.py key on the name.
repo_root=$(cd ../.. && pwd)
filter=$repo_root/common/src

# The media driver is excluded. The certification denominator is the portable C
# only, and driver glue is platform code outside it. The pattern removes exactly
# one file, common/src/fx_ram_driver.c, which nothing in the suite calls: the
# regression tests link a driver of their own instead. Nothing else under
# common/src has "driver" in its name.
exclude=".*driver.*"

# The coverage gate. No threshold is set yet: the figures these configurations
# reach are being measured for the first time, and a gate invented ahead of the
# measurement is a gate that either blocks a green tree or passes a regression.
# The thresholds are set from the set of lines and branch outcomes covered in
# *every* run of a sample taken on a clean runner, less a margin -- not from the
# lowest total any single run happened to report, which is a statistic about the
# sample rather than a statement about the tree.
min_line=${FX_COVERAGE_MIN_LINE:-0}
min_branch=${FX_COVERAGE_MIN_BRANCH:-0}

# --merge unions the per-configuration reports into the one number that means
# something. Each configuration writes an intermediate JSON beside its XML, and
# this pass adds them all together.
#
# Reporting eleven separate percentages instead would invite a reader to average
# them, and an average is not a coverage figure -- a line covered only by
# fault_tolerant_build_coverage is covered, and only the union says so.
#
# Do not be alarmed when the merged percentage is lower than the single
# configuration this used to report. That is the point: the denominator now
# includes code the old report never counted at all, because it was compiled out
# of the only instrumented build.
if [ "$1" = "--merge" ]; then
    shopt -s nullglob
    tracefiles=(coverage_report/per_configuration/*.json)
    shopt -u nullglob
    if [ ${#tracefiles[@]} -eq 0 ]; then
        echo "coverage.sh --merge: no JSON in coverage_report/per_configuration/." >&2
        echo "Run the suites with TX_COVERAGE=ON first." >&2
        exit 1
    fi

    add_args=()
    for t in "${tracefiles[@]}"; do
        add_args+=(--add-tracefile "$t")
    done

    mkdir -p coverage_report/merged
    gcovr -r "$repo_root" "${add_args[@]}" --xml-pretty --output coverage_report/merged.xml
    gcovr -r "$repo_root" "${add_args[@]}" --html --html-details --output coverage_report/merged/index.html

    if ! grep -q "<class " coverage_report/merged.xml; then
        echo "coverage.sh --merge: the merged report contains no files." >&2
        exit 1
    fi

    # Named, not just counted. coverage_report/ is not cleaned between runs, so a
    # tracefile left by an earlier run of a different set of configurations would
    # otherwise be merged in without anything saying so.
    echo "coverage.sh --merge: FileX, ${#tracefiles[@]} configuration(s):"
    for t in "${tracefiles[@]}"; do
        echo "    $(basename "$t" .json)"
    done

    # The exclusion is applied once, where the tracefiles are written, so the
    # merged report and the union below are computed from exactly the same
    # filtered data rather than from two filters that could drift apart.
    #
    # Run after the reports are written, so a failed gate still leaves behind the
    # reports that explain it.
    python3 "$(dirname "$(realpath "$0")")/coverage_union.py" \
            coverage_report/per_configuration "$min_line" "$min_branch" || exit $?
    exit 0
fi

# The positional search path is what scopes the report to one build
# configuration. --object-directory does not: with -r at the repository root
# gcovr still walks the whole tree, finds every configuration's gcda and reports
# the union, so all eleven configurations produce identical reports and --merge
# unions eleven copies of the same thing. It has to be absolute for the same reason
# -r does, since gcovr resolves it against the root.
objdir=$PWD/build/$1/filex/CMakeFiles/filex.dir/common/src

# Per-configuration output is kept in a subdirectory of its own, and the merged
# report sits alongside it at the top. That is not tidiness: the Pages deploy
# uploads coverage_report wholesale and merges every component's artifact into
# one tree, and configuration directories left at the top level would collide
# across components. Nested here, the top level holds exactly the merged report
# the deploy expects.
mkdir -p coverage_report/per_configuration/$1
gcovr -r "$repo_root" -f "$filter" -e "$exclude" "$objdir" \
      --json coverage_report/per_configuration/$1.json \
      --xml-pretty --output coverage_report/per_configuration/$1.xml
gcovr -r "$repo_root" -f "$filter" -e "$exclude" "$objdir" \
      --html --html-details --output coverage_report/per_configuration/$1/index.html

# An empty report is not an error as far as gcovr is concerned: it warns and
# exits 0. Worse, it advertises line-rate="1.0" alongside lines-valid="0", so
# every downstream consumer reads "no data at all" as "100% covered". A coverage
# threshold cannot catch that, because an empty report passes any threshold. So
# the assertion belongs here, next to the paths that would cause it.
if ! grep -q "<class " coverage_report/per_configuration/$1.xml; then
    echo "coverage.sh: the report for '$1' contains no files." >&2
    echo "Expected gcda files under $objdir." >&2
    exit 1
fi
