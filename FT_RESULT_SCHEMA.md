# FT Result Schema（模板）

> 目的：统一产测结果格式，支持自动判定、追溯分析、MES 对接与跨版本对比。

## 1. 顶层结构（建议）

```json
{
  "schema_version": "1.0.0",
  "run_id": "string",
  "dut_id": "string",
  "station_id": "string",
  "sdk_version": "string",
  "start_time": "ISO-8601",
  "end_time": "ISO-8601",
  "summary": {
    "total": 0,
    "pass": 0,
    "fail": 0,
    "blocked": 0,
    "overall_status": "PASS|FAIL|BLOCKED"
  },
  "cases": []
}
```

## 2. Case 结构（建议）

```json
{
  "case_id": "FT_BLE_SCAN_001",
  "case_name": "ble_scan_count_and_rssi",
  "status": "PASS|FAIL|BLOCKED",
  "bt_status": 0,
  "duration_ms": 1200,
  "retry_count": 1,
  "thresholds": {
    "min_device_count": 3,
    "min_rssi_dbm": -75
  },
  "metrics": {
    "device_count": 5,
    "rssi_avg_dbm": -62,
    "scan_latency_ms": 800
  },
  "verdict_reason": "string",
  "trace_id": "string",
  "raw_log_ref": "path_or_url"
}
```

## 3. 字段约束

- `schema_version`：遵循语义化版本。
- `run_id`：一次完整测试运行唯一标识。
- `case_id`：固定枚举，不可随意修改。
- `status`：仅允许 `PASS` / `FAIL` / `BLOCKED`。
- `bt_status`：保留底层错误码原值。
- `duration_ms`：非负整数。
- `metrics`：允许扩展字段，但核心字段需保持稳定。
- `trace_id`：必须可映射到设备日志链路。

## 4. 判定规则建议

1. 单 case 判定优先使用阈值与错误码联合规则。
2. `overall_status` 默认规则：
   - 存在 FAIL => FAIL
   - 无 FAIL 且存在 BLOCKED => BLOCKED
   - 全 PASS => PASS
3. BLOCKED 需有明确 `verdict_reason`（环境/依赖缺失）。

## 5. 错误码映射模板

| bt_status | 语义 | 建议判定 | 备注 |
|---|---|---|---|
| 0 | SUCCESS | PASS | - |
| 1 | FAIL | FAIL | 通用失败 |
| 2 | NOT_SUPPORTED | BLOCKED | 能力缺失 |
| 3 | TIMEOUT | FAIL | 可重试 |

> 注：请按项目 `bt_status.h` 实际定义补全并版本化维护。

## 6. 与阈值配置联动（建议）

- 阈值文件示例：`factory/config/test_thresholds.json`
- 运行时将阈值快照写入 `thresholds` 字段，确保结果可复现实验条件。
- 阈值变更必须记录版本与变更原因。

## 7. 上报与存储建议

1. 本地保存原始 JSON（不可覆盖，按 run_id 分文件）。
2. 上报前可附加签名字段（防篡改）。
3. MES 侧至少建立以下索引：`run_id`, `dut_id`, `station_id`, `overall_status`, `start_time`。

## 8. 向后兼容策略

- 新增字段：允许（消费者应忽略未知字段）。
- 删除/重命名字段：仅允许 MAJOR 升级。
- 字段语义变化：必须同步更新 `schema_version` 与迁移说明。
