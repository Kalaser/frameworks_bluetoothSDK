# factory_app

This directory is a runnable template corresponding to the plan in
`SDK_PORTING_PLAN_zh.md` section `6.4 生产测试应用落地模板`.

## Layout

- `main.c`: argument parsing and runner entry.
- `runner.c`: case orchestration and summary.
- `cases/case_smoke.c`: smoke FT cases.
- `report.c`: FT JSON report writer.
- `platform/platform_stub.c`: platform adaptation hook.
- `include/ft_runner.h`: shared FT data model and APIs.

## Build

```bash
make -C factory_app
```

## Run

```bash
./factory_app/ft_runner --run-id RUN-LOCAL-0001 --dut-id DUT-001 --station S01 --output /tmp/ft_result.json
python3 tools/test_suite/ft_report_validator.py /tmp/ft_result.json
```
