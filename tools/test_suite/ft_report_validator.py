#!/usr/bin/env python3
"""Validate FT report against repository schema baseline."""

from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_REPORT = ROOT / "factory" / "samples" / "ft_result.sample.json"

REQUIRED_TOP = {
    "schema_version",
    "run_id",
    "dut_id",
    "station_id",
    "sdk_version",
    "start_time",
    "end_time",
    "summary",
    "cases",
}
REQUIRED_SUMMARY = {"total", "pass", "fail", "blocked", "overall_status"}
REQUIRED_CASE = {
    "case_id",
    "case_name",
    "status",
    "bt_status",
    "duration_ms",
    "metrics",
    "verdict_reason",
    "trace_id",
}
VALID_STATUS = {"PASS", "FAIL", "BLOCKED"}


def _fail(msg: str) -> int:
    print(f"FT report validation failed: {msg}")
    return 1


def main() -> int:
    report = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else DEFAULT_REPORT
    if not report.exists():
        return _fail(f"report not found: {report}")

    data = json.loads(report.read_text(encoding="utf-8"))

    missing = REQUIRED_TOP - set(data.keys())
    if missing:
        return _fail(f"missing top-level keys: {sorted(missing)}")

    summary = data["summary"]
    if not isinstance(summary, dict):
        return _fail("summary must be object")
    missing_summary = REQUIRED_SUMMARY - set(summary.keys())
    if missing_summary:
        return _fail(f"missing summary keys: {sorted(missing_summary)}")

    if summary["overall_status"] not in VALID_STATUS:
        return _fail("summary.overall_status invalid")

    cases = data["cases"]
    if not isinstance(cases, list):
        return _fail("cases must be array")

    for idx, case in enumerate(cases):
        if not isinstance(case, dict):
            return _fail(f"cases[{idx}] must be object")
        missing_case = REQUIRED_CASE - set(case.keys())
        if missing_case:
            return _fail(f"cases[{idx}] missing keys: {sorted(missing_case)}")
        if case["status"] not in VALID_STATUS:
            return _fail(f"cases[{idx}].status invalid")

    print(f"FT report validation passed: {report}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
