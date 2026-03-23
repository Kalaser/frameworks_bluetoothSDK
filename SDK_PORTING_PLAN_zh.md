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

## 3. 创建可移植 SDK 的实施方案（分阶段）

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

---

## 4. 提高通用性的重点建议

### 4.1 能力协商优先于编译开关
- 保留 `CONFIG_*` 做裁剪，但对应用暴露运行时能力查询：
  - 是否支持 BR/EDR、LE、LE Audio、Codec、最大连接数等。
- 避免应用按宏条件编译，改为运行时分支。

### 4.2 统一异步与线程模型
- 当前样例已体现“回调线程 != 业务线程”的实践，建议沉淀为官方模式：
  1. callback 只入队；
  2. 业务线程消费；
  3. 所有同步 API 在非回调线程调用。
- SDK 提供默认事件循环适配器（pthread/uv/RTOS loop）。

### 4.3 Profile 能力模块化
- 将 A2DP/HFP/GATT/LEA 等做成可选子包（link-time / package-time）。
- 每个 profile 提供：
  - 初始化入口
  - 能力查询
  - 最小示例
  - 错误恢复指南

### 4.4 API 版本与兼容策略
- 采用 `major.minor.patch`：
  - major 变更允许 ABI break；
  - minor 仅新增接口；
  - patch 修复不改签名。
- 在头文件中引入 `BT_SDK_API_LEVEL`，支持老版本降级路径。

### 4.5 跨平台依赖隔离
- `libuv`、IPC、日志、存储等作为可替换适配层。
- 对 host Linux、RTOS、Android 三类平台提供不同默认实现。

---

## 5. 当前应用开发应怎么做（可立即落地）

## 5.1 推荐开发流程
1. **创建实例**：`bluetooth_create_instance()`。
2. **注册回调**：先注册 `adapter_callbacks_t`，再执行 enable/scan/connect。
3. **线程解耦**：回调线程只投递消息，业务线程处理状态机。
4. **按 profile 启停服务**：通过 `bluetooth_start_service/stop_service` 控制资源。
5. **退出清理**：取消回调、删除实例、清空消息队列。

## 5.2 推荐工程结构（应用侧）
- `bt_app_core.c`：实例管理、状态机、错误恢复。
- `bt_app_events.c`：回调到事件总线。
- `bt_app_profiles_*.c`：按 profile 分文件。
- `bt_app_hal.c`：平台差异（日志、线程、定时器）。

### 5.3 关键实践
- 对所有 `bt_status_t` 做统一检查和重试策略。
- 扫描、连接、配对流程设置超时与取消路径。
- 先以 `sample_code/basic`/`tests/adapter_test.c` 作为最小模板，逐步引入 profile。

---

## 6. 端口实践建议（给芯片/系统平台团队）

1. **先通 Adapter + Scan + Connect**，再逐个 profile 上线。
2. 建立 HCI 事件回放测试（利用 `debug/btsnoop_*`），用于端口回归。
3. 明确“栈能力 -> SAL 接口 -> SDK 能力”三段映射表，减少集成歧义。
4. 每新增后端至少通过：
   - 基础启停
   - BLE 扫描连接
   - GATT 读写
   - 一种音频 profile（A2DP 或 HFP）

---

## 7. 里程碑建议（务实版本）

- **M1（1个月）**：发布 v1 Lite SDK（Adapter+Scan+GATTc），含文档与示例。
- **M2（2个月）**：完成第二后端（非 Zephyr）并通过兼容性测试。
- **M3（3个月）**：发布完整 profile SDK 与应用开发手册、错误码手册、迁移指南。

---

## 8. 结论

这个仓库已经具备“可移植 SDK”核心雏形（API 层 + Core 层 + SAL 层），下一步重点不是“重写”，而是：
1. **冻结公共边界**，
2. **强化后端插件化和能力协商**，
3. **统一应用异步开发模型**，
4. **把示例和测试产品化为 SDK 交付物**。

这样可以同时提升：
- 对不同协议栈/芯片平台的可移植性；
- 对第三方应用开发者的可用性；
- 对长期演进的兼容性与维护效率。
