#!/usr/bin/env python3
"""Validate capability matrix used by runtime capability negotiation."""

from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MATRIX_PATH = ROOT / "factory" / "config" / "capability_matrix.json"
REQUIRED_GLOBAL_KEYS = ["schema_version", "backends"]
REQUIRED_BACKEND_KEYS = [
    "backend_id",
    "stack",
    "transport",
    "profiles",
    "limits",
]


def main() -> int:
    if not MATRIX_PATH.exists():
        print(f"Missing capability matrix: {MATRIX_PATH}")
        return 1

    data = json.loads(MATRIX_PATH.read_text(encoding="utf-8"))
    for key in REQUIRED_GLOBAL_KEYS:
        if key not in data:
            print(f"Missing top-level key: {key}")
            return 1

    backends = data.get("backends")
    if not isinstance(backends, list) or not backends:
        print("'backends' must be a non-empty array")
        return 1

    backend_ids: set[str] = set()
    for idx, backend in enumerate(backends):
        if not isinstance(backend, dict):
            print(f"backend[{idx}] must be an object")
            return 1
        for key in REQUIRED_BACKEND_KEYS:
            if key not in backend:
                print(f"backend[{idx}] missing key: {key}")
                return 1

        bid = backend["backend_id"]
        if bid in backend_ids:
            print(f"duplicate backend_id: {bid}")
            return 1
        backend_ids.add(bid)

        for arr_key in ("transport", "profiles"):
            if not isinstance(backend[arr_key], list):
                print(f"backend[{idx}].{arr_key} must be an array")
                return 1

        limits = backend["limits"]
        if not isinstance(limits, dict) or "max_connections" not in limits:
            print(f"backend[{idx}].limits.max_connections is required")
            return 1

    print("Capability matrix check passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
