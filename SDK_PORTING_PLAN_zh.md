# BluetoothSDK 仓库分析与 SDK 化移植方案（面向通用平台与应用开发）

## 1. 当前仓库架构快速分析

### 1.1 分层结构（已具备 SDK 雏形）
- **框架 API 层（`framework/include` + `framework/api`）**
  - 对外暴露稳定 C 接口（如 `bluetooth_create_instance`、`bt_adapter_*`、`bt_gatt*`、`bt_a2dp*`）。
  - `framework/api/*.c` 基本作为 facade/薄封装，把请求转到内部 manager/adapter/profile。
- **服务层（`service/src` + `service/profiles`）**
  - `manager_service` 负责实例生命周期、app_id 分配、服务控制。
  - `service_manager` 负责 Profile 注册、启动、停机、消息分发。
  - 各 profile（A2DP/HFP/GATT/LEA/...）以 `profile_service_t` 插件式注册。
- **栈抽象层 SAL（`service/stacks/include/sal_*.h`）**
  - 定义“统一协议栈能力接口”，上层不直接依赖 Zephyr/BlueZ。
  - 当前已有 Zephyr 侧实现（`service/stacks/zephyr`）。
- **Feature/QuickApp 层（`feature`）**
  - auto-generated 的系统特性包装，将蓝牙能力暴露给 QuickApp/JS 运行时。
- **样例与测试（`sample_code`、`tests`）**
  - 样例展示“创建实例 + 注册回调 + 主线程消息处理”的标准开发路径。

### 1.2 已有优势
1. **跨栈抽象已经存在**：SAL 接口覆盖 Adapter/LE/GATT/Profile 能力，便于新增后端。
2. **Profile 插件化管理**：通过 `register_service` + `service_manager_*` 统一生命周期。
3. **多入口支持**：C NDK API、Feature API、IPC（socket/binder 目录）。

### 1.3 当前限制（影响“可移植 SDK”）
1. **公共 API 与实现耦合仍偏紧**：`framework/api` 对内部 manager 依赖强，适配层边界还可收紧。
2. **异步模型分散**：callback、service_loop、uv、feature runtime 并存，应用接入成本偏高。
3. **配置能力以编译期开关为主**：大量 `CONFIG_*`，运行时能力发现/协商不够统一。
4. **错误码与可观测性未体系化**：跨层透传/映射规则不够统一，给上层做容错较难。

---

## 2. SDK 化目标（建议）

建议把仓库演进为“**三层两边界**”SDK：

- **Public SDK（稳定 ABI）**：`framework/include` + `framework/api`，只暴露应用视角能力。
- **Core Runtime（可替换实现）**：`service/src` + `service/profiles`，负责状态机与策略。
- **Stack Adapter（可插拔后端）**：`service/stacks/*`，新增 `zephyr/bluez/vendor_xxx` 实现。

两条边界：
1. **Public API 边界**：禁止应用直接 include `service/*` 私有头。
2. **Stack Adapter 边界**：core 仅依赖 `sal_*` 接口，不依赖具体栈类型。

---

## 3. 面向“生产测试（PVT/产线）”的 SDK 设计与开发方案

如果你的目标是“在生产测试中可批量使用”的 SDK，建议在通用 SDK 之外，增加一层 **Factory Test SDK（FT-SDK）**，并坚持“能力收敛、流程固定、结果可追溯”三原则。

### 3.1 FT-SDK 分层建议（在现有框架上最小增量）

在现有三层架构上增加一个面向产测的上层：

- **FT API 层（新增）**
  - 提供稳定且短小的测试接口（如：射频读写、扫描 KPI、连接 KPI、GATT 回环、功耗采样触发）。
  - 返回统一测试结果对象（状态码、耗时、原始指标、判定结论）。
- **Public SDK 层（现有 `framework/include` + `framework/api`）**
  - 复用 `bluetooth_create_instance`、`bt_adapter_*`、`bt_gatt*` 等基础能力。
- **Core Runtime 层（现有 `service/src` + `service/profiles`）**
  - 复用实例管理、profile 生命周期、消息分发。
- **Stack Adapter 层（现有 `service/stacks/*`）**
  - 复用 SAL，适配不同协议栈/芯片。

### 3.2 产测 SDK 最小能力集（建议先做）

建议先做以下 10 个原子测试项，形成 `ft_suite_smoke`：

1. 适配器上电/下电（含超时）
2. 本机地址读取与校验
3. BLE 扫描（数量阈值、RSSI 阈值）
4. BLE 定向连接（成功率、建链时延）
5. GATT 读写回环（数据一致性、往返时延）
6. BR/EDR Inquiry（如果机型支持）
7. 配对/解绑（成功率）
8. 断链重连（N 次稳定性）
9. 发射/接收测试模式入口（若芯片支持 VSC）
10. 日志与结果落盘（本地 + 上传）

### 3.3 统一结果模型（强烈建议）

建议定义统一结构体（示意）：

- `case_id`：测试项编号（固定枚举）
- `status`：PASS/FAIL/BLOCKED
- `bt_status`：底层错误码（原值）
- `duration_ms`：耗时
- `metrics`：关键指标（如 rssi_avg、conn_latency_ms、retry_cnt）
- `verdict_reason`：失败原因（可读）
- `trace_id`：关联日志链路（便于追溯）

产线要的是“可判定”和“可追责”，不是只有 API 返回 0/1。

### 3.4 产测流程编排（推荐状态机）

建议实现固定编排器：

1. 环境预检（电源、串口、天线、权限）
2. 蓝牙初始化
3. 用例顺序执行（支持 fail-fast / continue-on-fail）
4. 结果聚合（单项 + 总结论）
5. 资源回收（反注册、实例释放、日志归档）

并提供两种执行模式：
- **在线模式**：与 MES/产测上位机实时交互；
- **离线模式**：本地执行后导出 JSON 报告。

---

## 4. 创建可移植 SDK 的实施方案（分阶段）

## 阶段 A：接口冻结与打包（2~4 周）
1. 冻结 `framework/include` 为 v1 SDK 头文件集。
2. 建立导出符号清单（例如 `BTSYMBOLS` + map 文件），保证 ABI 稳定。
3. 生成 `libbluetoothsdk.so/.a` + `pkg-config`/CMake config。
4. 增加“最小能力子集”（Adapter + Scan + GATT Client）作为 Lite SDK。

**产出**：
- `include/`（公共头）
- `lib/`（动态/静态库）
- `samples/`（最小 demo）
- `sdk_manifest.json`（版本、能力、编译选项）

## 阶段 B：后端插件化与平台端口（4~8 周）
1. 规范 SAL 适配模板：每个后端必须实现的能力矩阵（必选/可选）。
2. 新增 `stack_backend_registry`：按配置或运行时选择后端。
3. 引入 capability 查询 API（例如 `bt_adapter_get_capabilities`）。
4. 为 `BlueZ` 或目标芯片厂商栈实现第二后端，验证“同 API 跨栈”。

**产出**：
- `service/stacks/template`（端口模板）
- `docs/porting_guide.md`（从 HCI/事件映射到 SAL 的映射清单）

## 阶段 C：应用开发体验优化（3~6 周）
1. 提供统一异步模型（推荐事件队列 + future/promise 封装二选一）。
2. 提供 profile 级 Session API（减少应用直接处理底层状态机）。
3. 增加“错误码域 + 可恢复建议”。
4. 完善 mock backend + CI 场景测试（无真实蓝牙硬件也可回归）。

## 阶段 D：生产测试能力固化（2~4 周，可与阶段 C 并行）
1. 增加 FT-SDK API（建议独立头文件：`bt_factory_test.h`）。
2. 建立测试项注册机制（case registry），支持机型裁剪。
3. 增加阈值配置（JSON/ini）与版本化管理（随固件版本绑定）。
4. 落地结果上报协议（本地文件 + socket/http 上报适配）。
5. 为关键 case 提供 golden-device 对照测试。

**产出**：
- `factory/include/bt_factory_test.h`
- `factory/src/*`（编排器、测试项实现、结果聚合）
- `factory/config/test_thresholds.json`
- `tools/test_suite` 对接脚本

---

## 5. 提高通用性的重点建议

### 5.1 能力协商优先于编译开关
- 保留 `CONFIG_*` 做裁剪，但对应用暴露运行时能力查询：
  - 是否支持 BR/EDR、LE、LE Audio、Codec、最大连接数等。
- 避免应用按宏条件编译，改为运行时分支。

### 5.2 统一异步与线程模型
- 当前样例已体现“回调线程 != 业务线程”的实践，建议沉淀为官方模式：
  1. callback 只入队；
  2. 业务线程消费；
  3. 所有同步 API 在非回调线程调用。
- SDK 提供默认事件循环适配器（pthread/uv/RTOS loop）。

### 5.3 Profile 能力模块化
- 将 A2DP/HFP/GATT/LEA 等做成可选子包（link-time / package-time）。
- 每个 profile 提供：
  - 初始化入口
  - 能力查询
  - 最小示例
  - 错误恢复指南

### 5.4 API 版本与兼容策略
- 采用 `major.minor.patch`：
  - major 变更允许 ABI break；
  - minor 仅新增接口；
  - patch 修复不改签名。
- 在头文件中引入 `BT_SDK_API_LEVEL`，支持老版本降级路径。

### 5.5 跨平台依赖隔离
- `libuv`、IPC、日志、存储等作为可替换适配层。
- 对 host Linux、RTOS、Android 三类平台提供不同默认实现。

---

## 6. 当前应用开发应怎么做（可立即落地）

## 6.1 推荐开发流程
1. **创建实例**：`bluetooth_create_instance()`。
2. **注册回调**：先注册 `adapter_callbacks_t`，再执行 enable/scan/connect。
3. **线程解耦**：回调线程只投递消息，业务线程处理状态机。
4. **按 profile 启停服务**：通过 `bluetooth_start_service/stop_service` 控制资源。
5. **退出清理**：取消回调、删除实例、清空消息队列。

## 6.2 推荐工程结构（应用侧）
- `bt_app_core.c`：实例管理、状态机、错误恢复。
- `bt_app_events.c`：回调到事件总线。
- `bt_app_profiles_*.c`：按 profile 分文件。
- `bt_app_hal.c`：平台差异（日志、线程、定时器）。

### 6.3 关键实践
- 对所有 `bt_status_t` 做统一检查和重试策略。
- 扫描、连接、配对流程设置超时与取消路径。
- 先以 `sample_code/basic`/`tests/adapter_test.c` 作为最小模板，逐步引入 profile。

## 6.4 生产测试应用落地模板（建议）

建议你的产测程序按如下目录组织：

- `factory_app/main.c`：参数解析、测试套执行入口
- `factory_app/runner.c`：用例编排与超时控制
- `factory_app/cases/case_*.c`：原子测试项
- `factory_app/report.c`：结果聚合、JSON 输出、上报
- `factory_app/platform/*.c`：串口/文件/时间戳/网络适配

建议 CLI 示例：

- `ft_runner --suite smoke --dut-id xxx --station S01 --output /tmp/ft.json`

---

## 7. 端口实践建议（给芯片/系统平台团队）

1. **先通 Adapter + Scan + Connect**，再逐个 profile 上线。
2. 建立 HCI 事件回放测试（利用 `debug/btsnoop_*`），用于端口回归。
3. 明确“栈能力 -> SAL 接口 -> SDK 能力”三段映射表，减少集成歧义。
4. 每新增后端至少通过：
   - 基础启停
   - BLE 扫描连接
   - GATT 读写
   - 一种音频 profile（A2DP 或 HFP）

---

## 8. 里程碑建议（务实版本）

- **M1（1个月）**：发布 v1 Lite SDK（Adapter+Scan+GATTc）+ `ft_suite_smoke`。
- **M2（2个月）**：完成第二后端（非 Zephyr）并通过兼容性测试 + 产测阈值固化。
- **M3（3个月）**：发布完整 profile SDK + FT-SDK + 产线接入手册。

---

## 9. 结论

这个仓库已经具备“可移植 SDK”核心雏形（API 层 + Core 层 + SAL 层），下一步重点不是“重写”，而是：
1. **冻结公共边界**，
2. **强化后端插件化和能力协商**，
3. **统一应用异步开发模型**，
4. **把示例和测试产品化为 SDK 交付物**。

这样可以同时提升：
- 对不同协议栈/芯片平台的可移植性；
- 对第三方应用开发者的可用性；
- 对长期演进的兼容性与维护效率。

---

## 10. 四项重点的工程化落地清单（可直接执行）

### 10.1 冻结公共边界（Public SDK Boundary Freeze）
**目标**：让应用只依赖稳定 API，不感知 `service/*` 内部实现变化。
**动作**：
1. 固化公共头文件白名单（仅 `framework/include/*.h`）。
2. 生成并维护导出符号清单（按版本对比）。
3. 新增 CI 检查：禁止在 sample/app 中 include `service/*` 私有头。
4. 建立 API 变更评审模板（兼容性说明 + 替代路径）。
**验收**：
- 通过 ABI 检查（版本升级后旧应用二进制可运行）。
- 公共头文件 diff 与符号 diff 可追溯。

### 10.2 强化后端插件化和能力协商（Backend Plugin + Capability Negotiation）
**目标**：同一上层 API 可在不同协议栈/芯片后端运行。
**动作**：
1. 建立后端注册表（`backend_id -> sal_ops`）。
2. 输出能力位图（BR/EDR、LE、LEA、VSC、最大连接数等）。
3. 所有 profile 启动前先做 capability gate（不支持则返回统一错误码）。
4. 至少维护两套后端实现（当前后端 + 对照后端）持续回归。
**验收**：
- 同一 smoke 用例可在两套后端跑通。
- 能力不满足时错误码、日志、上层提示一致。

### 10.3 统一应用异步开发模型（Unified Async Model）
**目标**：降低多线程/回调使用复杂度，减少竞态问题。
**动作**：
1. 规范回调线程职责：只入队，不做耗时逻辑。
2. 提供统一事件循环适配层（pthread/uv/RTOS 统一接口）。
3. 提供同步 API 调用上下文约束（文档 + 运行时断言/日志）。
4. 输出标准事件序列文档（enable->scan->connect->pair->profile up/down）。
**验收**：
- 示例和测试全部按统一事件模型运行。
- 关键流程无死锁/重入问题（压测稳定）。

### 10.4 把示例和测试产品化为 SDK 交付物（Productized Samples & Tests）
**目标**：SDK 交付后可快速接入、快速验收、快速定位问题。
**动作**：
1. 沉淀最小示例包：`basic / discovery / createbond / ft_suite_smoke`。
2. 测试产物标准化：统一 JSON 报告格式（含 trace_id）。
3. 增加 host 侧快速自测命令与文档（开箱即跑）。
4. 示例、测试与 SDK 版本绑定发布（版本号一致）。
**验收**：
- 新平台可在 1 天内完成 SDK 冒烟验证。
- 问题可通过报告和 trace_id 追溯到具体 case 与日志。
