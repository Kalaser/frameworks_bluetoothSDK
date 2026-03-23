#!/usr/bin/env python3
"""Check that external-facing code does not include private service headers."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SCAN_DIRS = [
    ROOT / "sample_code",
    ROOT / "tests",
    ROOT / "tools",
]
ALLOWED_PREFIXES = (
    "framework/include/",
    "framework/common/",
)
PATTERN = re.compile(r'#\s*include\s*[<\"]([^>\"]+)[>\"]')


def should_scan(path: Path) -> bool:
    return path.suffix in {".c", ".cc", ".cpp", ".h", ".hpp"}


def is_private_include(include_path: str) -> bool:
    if include_path.startswith(ALLOWED_PREFIXES):
        return False
    return include_path.startswith("service/")


def main() -> int:
    violations: list[str] = []
    for scan_dir in SCAN_DIRS:
        if not scan_dir.exists():
            continue
        for file in scan_dir.rglob("*"):
            if not file.is_file() or not should_scan(file):
                continue
            rel = file.relative_to(ROOT)
            try:
                text = file.read_text(encoding="utf-8", errors="ignore")
            except OSError as exc:
                violations.append(f"{rel}: read error: {exc}")
                continue

            for lineno, line in enumerate(text.splitlines(), start=1):
                match = PATTERN.search(line)
                if not match:
                    continue
                inc = match.group(1)
                if is_private_include(inc):
                    violations.append(
                        f"{rel}:{lineno}: private include is not allowed in external code: {inc}"
                    )

    if violations:
        print("Public boundary check failed:")
        for item in violations:
            print(f"  - {item}")
        return 1

    print("Public boundary check passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
