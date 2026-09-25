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

# The coverage gate, set from the set of lines and branch outcomes covered in
# *every* run of a sample taken on a clean runner -- not from the lowest total
# any single run happened to report, which is a statistic about the sample
# rather than a statement about the tree.
#
# The union measures 7805 of 7805 lines and 4797 of 4798 branch outcomes, and
# the values below are those figures truncated to two decimal places. Line
# coverage is at 100.00%: every line of the certified source is reached in at
# least one supported configuration, and the gate holds it there. The single
# uncovered branch outcome is fx_partition_offset_calculate.c:520 ordinal 1, the
# one construct carried as a deviation with its attempt log; it is reachable
# only by a caller that no partition table can produce.
#
# The denominator is common/src over every build configuration, less the media
# driver above and less the code the regression-test hook macros inject into it
# -- 11 lines and 40 branch outcomes at 11 sites, all of them covered, which an
# application does not compile at all. coverage_union.py holds the list and
# prints it with the figure. Removing it moves neither threshold: 4797 of 4798
# is 99.979158% where 4837 of 4838 was 99.979330%, and both truncate to the
# 99.97 below, so the exclusion and the gate are independent of each other.
#
# The unstable set is empty. It has had exactly one member in the life of this
# branch -- fx_utility_logical_sector_read.c:346 ordinal 3, the multi-sector arm
# of a read starting at the sector already in the media memory buffer, which was
# seen covered once by no test that was aiming at it and in no observation
# since. It is now covered deliberately, by a test that reaches it in all four
# configurations that compile it, so it is part of the always-covered set on the
# same terms as every other key rather than by luck. No key on this tree has
# ever been observed leaving that set.
#
# The coverage work has closed 80 lines and 103 outcomes since the gate was
# first set, and the gate is raised with it rather than left behind as slack. It
# still fails on a single line or a single outcome going missing.
#
# Truncated rather than rounded, and the report truncates with it, so the two
# now agree by construction. The comparison is made against the full-precision
# ratio, so a threshold taken from a figure rounded *up* fails a run in which
# nothing has regressed. That has now been reachable three times: 7736 of 7816
# lines is 98.976459%, which read as 98.98 when the gate was first set at 98.97;
# 4817 of 4838 outcomes is 99.565936%, which read as 99.57 against a gate of
# 99.56; and the branch figure here is 99.979158%, which reads as 99.98 against
# the 99.97 below -- as did the 99.979330% it replaced, so the hook exclusion
# changes nothing about this. The first two were caught by hand. The third was
# not reachable by hand at all, because coverage_union.py now rounds its own
# output down and printed the figure the gate takes -- which is what the change
# was for.
#
# There is no margin below that, and an empty unstable set is what a zero margin
# rests on. What a margin protects against is an outcome moving from *always
# covered* to *unstable*, which is the only movement that can fail this gate on
# a run in which nothing regressed. Neither movement seen on this tree is that
# one: a key went from *never* to *unstable*, which can only add coverage above
# the floor, and then from *unstable* to *always* when a test closed it. If a
# key on this tree is ever observed leaving the always-covered set, that is the
# point at which this paragraph has to be rewritten and a margin measured.
#
# The rest is redundancy depth, and it is why the floor sits where it does.
# 7626 of the 7805 covered lines and 4747 of the 4797 covered outcomes are
# reached by two or more configurations, so a miss in one is absorbed by a
# sibling. Every excluded hook site was in that set -- each is covered by every
# configuration that compiles it -- so the exclusion came out of the redundant
# share and left the counts below untouched. The remaining 179 lines and 50
# outcomes are covered by exactly one configuration, and for 178 of those lines
# and all 50 outcomes that configuration is also the only one that compiles the
# key, where coverage is deterministic: the code exists there or nowhere. That
# leaves exactly one key compiled by more than one configuration and covered by
# only one -- fx_file_write.c:886, the unprotect on the sector write error path,
# held by no_cache_fault_tolerant_build, which has not moved across any sample
# taken.
#
# The outcome count in that single-configuration set grew from 32 to 50 with the
# trace shim coverage, and the growth is not fragility. Those 18 outcomes are
# the arms of the two trace shims and of the two system getters' trace guard,
# and trace_build_coverage is the only configuration that compiles any of them.
#
# The gate reads the union figure from coverage_union.py, not the percentage in
# the merged report. gcovr's merge keys each branch by the basic-block pair gcov
# gave it, and those numbers shift when a file compiles to a different amount of
# code, so the same source branch is counted once per configuration that
# renumbers it. Here that inflates the merged branch denominator from 4838 to
# 10109. Both figures are produced and the merged report is published unchanged;
# the gate uses the union because it is the one that counts branches in the
# source, and therefore the one that does not lurch when a configuration is
# added.
#
# The line axis is now at the target rather than on the way to it, and the
# branch axis is one deviation short of it. A line gate of 100.00 passes only
# while nothing regresses at all, which is the point of setting it there.
min_line=${FX_COVERAGE_MIN_LINE:-100.00}
min_branch=${FX_COVERAGE_MIN_BRANCH:-99.97}

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
