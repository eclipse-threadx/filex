#!/usr/bin/env python3
"""Report coverage of the certified source, unioned across build configurations.

gcovr's --add-tracefile merge keys every branch by the basic-block pair gcov
assigned it. Those block numbers are not a property of the source: a file that
compiles to a different amount of code in two configurations gets its blocks
renumbered, so the same source branch arrives under two identities and the merge
counts it twice. FileX has every reason to meet this hard: eleven
build configurations, and FX_ENABLE_FAULT_TOLERANT alone compiles twenty files that no
other configuration compiles at all.

That inflation is symmetric for a branch covered everywhere, so the percentage
stays plausible and the fault is easy to miss. It is not harmless: the figure a
certification report quotes as "branches in the certified source" has to be
branches in the certified source, and the number grows or shrinks when a build
configuration is added for reasons that have nothing to do with the code.

This script reports the union instead. A line is covered if any configuration
executed it; the Nth branch on a line is covered if any configuration took it.
Branches are keyed by their position within the line rather than by block
number, which is stable across configurations because the order gcov emits them
in follows the source expression.

Both figures are kept. gcovr's merged report is still produced and published
unchanged -- it is the tool's own output and nothing here rewrites it. This is
the figure the coverage ratchet gates on, so that adding a build configuration
moves the number only by the code it actually brings in.

The union also drops the code the regression-test hook macros inject into the
certified source. See HOOK_SITES below for what that is and why it is not part
of the denominator; the excluded sites are printed with the figure, because an
exclusion nobody can see in the output is an exclusion nobody can audit.
"""

import glob
import json
import math
import os
import sys


# The regression-test hook code, which is not part of the certified denominator.
#
# CMakeLists.txt puts -DFX_REGRESSION_TEST on every build configuration. That
# opens a block in the Linux port header, ports/linux/gnu/inc/fx_port.h lines
# 116-263, which defines ten *_EXTENSION macros that expand to live code -- a
# counter, a flag test, a decrement and in most cases an early
# return(FX_IO_ERROR) -- at the eleven sites in common/src carried by this list.
# An application does not define the macro, fx_api.h's and fx_fault_tolerant.h's
# #ifndef fallbacks then expand every one of them to nothing, and none of this
# code exists in a build anyone ships. gcov attributes a macro expansion to the
# file that invokes it, so without this list the harness's own scaffolding is
# counted as certified source.
#
# Only a macro the *test build* defines belongs here. A macro the port header
# defines is in the application's build too, so removing its code would take
# shipped code out of the denominator -- the opposite error. FX_REGRESSION_TEST
# is the only macro in this tree that meets the test: it is defined in exactly
# one place, test/cmake/CMakeLists.txt, and the port header only tests it.
#
# The line numbers are stable because the certified baseline is a frozen tag,
# and a site that moves is caught rather than silently skipped: every entry must
# match a line the report carries, or this fails.
HOOK_SITES = [
    ("common/src/fx_directory_entry_read.c", 107, "FX_DIRECTORY_ENTRY_READ_EXTENSION"),
    ("common/src/fx_directory_entry_write.c", 112, "FX_DIRECTORY_ENTRY_WRITE_EXTENSION"),
    ("common/src/fx_fault_tolerant_apply_logs.c", 113, "FX_FAULT_TOLERANT_APPLY_LOGS_EXTENSION"),
    ("common/src/fx_fault_tolerant_enable.c", 408, "FX_FAULT_TOLERANT_ENABLE_EXTENSION"),
    ("common/src/fx_fault_tolerant_enable.c", 420, "FX_FAULT_TOLERANT_ENABLE_EXTENSION"),
    ("common/src/fx_utility_FAT_entry_read.c", 124, "FX_UTILITY_FAT_ENTRY_READ_EXTENSION"),
    ("common/src/fx_utility_FAT_entry_write.c", 130, "FX_UTILITY_FAT_ENTRY_WRITE_EXTENSION"),
    ("common/src/fx_utility_logical_sector_flush.c", 98, "FX_UTILITY_LOGICAL_SECTOR_FLUSH_EXTENSION"),
    ("common/src/fx_utility_logical_sector_read.c", 133, "FX_UTILITY_LOGICAL_SECTOR_READ_EXTENSION"),
    ("common/src/fx_utility_logical_sector_read.c", 520, "FX_UTILITY_LOGICAL_SECTOR_READ_EXTENSION_1"),
    ("common/src/fx_utility_logical_sector_write.c", 122, "FX_UTILITY_LOGICAL_SECTOR_WRITE_EXTENSION"),
]

# Sites where a hook displaces shipped code rather than occupying an empty line,
# so that only its branches may come out. This port has none, and the empty list
# is the record that the question was asked of all ten macros rather than
# assumed: each one's #ifndef fallback was read, and every one of them is empty
# -- the eight in fx_api.h:646-679 and the two fault-tolerant ones in
# fx_fault_tolerant.h:72-79. Without the hook each invocation line therefore
# holds nothing at all, and the whole line goes.
#
# A non-empty fallback would mean the line still carries a statement in a build
# that ships, and excluding it would remove shipped code from the denominator.
# ThreadX has exactly that case in TX_TIMER_INITIALIZE_EXTENSION, whose fallback
# under TX_MISRA_ENABLE is a real assignment; this port's equivalent slot is
# empty, and a port header that later defines one of these macros outside the
# FX_REGRESSION_TEST block would belong here rather than above.
HOOK_BRANCH_SITES = []


def exclude_hook_sites(lines, branches):
    """Drop the hook expansions from the union, and report what was dropped.

    Returns one row per site: the macro, and the covered/total it took out of
    each axis. Covered/total rather than a count, so the output shows on its
    face that the exclusion removed nothing that was uncovered -- which would
    raise the figure for the wrong reason.
    """

    report = []
    missing = []

    for path, number, macro, drop_line in (
            [(p, n, m, True) for p, n, m in HOOK_SITES] +
            [(p, n, m, False) for p, n, m in HOOK_BRANCH_SITES]):

        outcomes = sorted(k for k in branches if k[0] == path and k[1] == number)
        if (path, number) not in lines:
            missing.append((path, number, macro))
            continue

        line_covered = line_total = 0
        if drop_line:
            line_covered, line_total = lines.pop((path, number)), 1

        outcome_covered = sum(branches.pop(k) for k in outcomes)
        report.append((path, number, macro, drop_line,
                       line_covered, line_total, outcome_covered, len(outcomes)))

    return report, missing


def union(tracefiles):
    """Union line and branch coverage across per-configuration tracefiles."""

    lines = {}
    branches = {}

    for path in tracefiles:
        with open(path, encoding="utf-8") as handle:
            data = json.load(handle)

        for entry in data.get("files", []):
            name = entry["file"]
            for line in entry.get("lines", []):
                number = line["line_number"]
                key = (name, number)
                lines[key] = lines.get(key, 0) or (1 if line["count"] > 0 else 0)

                for index, branch in enumerate(line.get("branches", [])):
                    key = (name, number, index)
                    branches[key] = branches.get(key, 0) or (1 if branch["count"] > 0 else 0)

    return lines, branches


def truncate(rate):
    """Round a percentage down to the two decimal places the report prints.

    Rounding to nearest would print a figure above the ratio it stands for: 7736 of
    7816 lines is 98.976459%, which reads as 98.98, and 4817 of 4838 branch outcomes
    is 99.565936%, which reads as 99.57. A coverage report that overstates coverage,
    even by a hundredth, is the wrong error for certification evidence to make.

    It also keeps the report and the gate in step. The threshold in coverage.sh is
    compared against the full-precision ratio, so a gate set to a rounded-up figure
    fails a tree in which nothing has regressed -- which is what both figures above
    would have caused, and what each cost a reader's attention to catch by hand.
    Truncating here makes the printed figure the one a threshold can safely be set to.
    """

    return math.floor(rate * 100) / 100


def main():
    if len(sys.argv) < 4:
        print("usage: coverage_union.py <tracefile-dir> <min-line> <min-branch>", file=sys.stderr)
        return 2

    directory, min_line, min_branch = sys.argv[1], float(sys.argv[2]), float(sys.argv[3])

    tracefiles = sorted(glob.glob(os.path.join(directory, "*.json")))
    if not tracefiles:
        print("coverage_union.py: no JSON in %s." % directory, file=sys.stderr)
        print("Run the suites with TX_COVERAGE=ON first.", file=sys.stderr)
        return 1

    lines, branches = union(tracefiles)
    if not lines:
        print("coverage_union.py: the tracefiles contain no files.", file=sys.stderr)
        return 1

    excluded, missing = exclude_hook_sites(lines, branches)
    if missing:
        for path, number, macro in missing:
            print("coverage_union.py: %s:%d is not in the report -- %s has moved."
                  % (path, number, macro), file=sys.stderr)
        print("coverage_union.py: re-derive HOOK_SITES from the port header.", file=sys.stderr)
        return 1

    line_covered, line_total = sum(lines.values()), len(lines)
    branch_covered, branch_total = sum(branches.values()), len(branches)

    line_rate = (100.0 * line_covered) / line_total
    branch_rate = (100.0 * branch_covered) / branch_total if branch_total else 100.0

    print("coverage_union.py: unioned over %d configuration(s):" % len(tracefiles))
    for path in tracefiles:
        print("    %s" % os.path.basename(path)[:-len(".json")])

    print("    excluded, regression-test hook expansions (-DFX_REGRESSION_TEST):")
    hook_lines_covered = hook_lines_total = 0
    hook_outcomes_covered = hook_outcomes_total = 0
    for path, number, macro, drop_line, lc, lt, oc, ot in excluded:
        print("        %-40s lines %d/%d, outcomes %2d/%-2d  %s%s"
              % ("%s:%d" % (path[len("common/src/"):], number),
                 lc, lt, oc, ot, macro, "" if drop_line else ", branches only"))
        hook_lines_covered += lc
        hook_lines_total += lt
        hook_outcomes_covered += oc
        hook_outcomes_total += ot
    print("        %-40s lines %d/%d, outcomes %2d/%-2d"
          % ("total", hook_lines_covered, hook_lines_total,
             hook_outcomes_covered, hook_outcomes_total))

    print("    lines    %d/%d - %.2f%%" % (line_covered, line_total, truncate(line_rate)))
    print("    branches %d/%d - %.2f%%" % (branch_covered, branch_total, truncate(branch_rate)))

    status = 0
    if line_rate < min_line:
        print("coverage_union.py: failed minimum line coverage (got %.2f%%, minimum %.2f%%)"
              % (truncate(line_rate), min_line), file=sys.stderr)
        status = 1
    if branch_rate < min_branch:
        print("coverage_union.py: failed minimum branch coverage (got %.2f%%, minimum %.2f%%)"
              % (truncate(branch_rate), min_branch), file=sys.stderr)
        status = 1

    return status


if __name__ == "__main__":
    sys.exit(main())
