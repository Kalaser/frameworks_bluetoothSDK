#!/usr/bin/env bash
set -euo pipefail

python3 tools/test_suite/check_public_boundary.py
python3 tools/test_suite/check_capability_matrix.py
python3 tools/test_suite/ft_report_validator.py
