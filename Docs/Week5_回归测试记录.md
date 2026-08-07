# Week5 回归测试记录

生成时间：2026-05-26

## 1. Test Scope

本文件是第 5 周网络权威化的回归测试记录模板。当前状态为周一审计后创建，尚未执行实际两客户端测试。

测试目标来自 `Week5_网络职责表.md`：

1. 射击、扣弹、命中、扣血由服务器决定。
2. 死亡、复活、击杀数、死亡数由服务器决定。
3. 拾枪、丢枪、当前武器引用、武器归属由服务器决定。
4. `RemainingTime`、`gameOver` 等比赛状态由 GameMode 写入 GameState，并复制给客户端。
5. Multicast 只播放动画、声音、特效、显示隐藏、UI 刷新等表现。

测试环境待填写：

| 项目 | 值 |
| :--- | :--- |
| UE 版本 | 4.27 |
| 地图 | `/Game/Season7/Main.Main` |
| GameMode | `/Game/All/BP_MyGamemode.BP_MyGamemode_C` |
| 服务器模式 | 独立服务器 / Listen Server / PIE，待填写 |
| 客户端数量 | 2 |
| 测试人 | 待填写 |
| 测试日期 | 待填写 |

## 2. TDD Checklist

| 检查项 | 状态 | 备注 |
| :--- | :--- | :--- |
| 6 个核心资产 JSON 已导出 | Pass | 输出目录 `C:\Users\19370\.local\share\opencode\uasset-json\project10` |
| 已形成 20+ 节点/函数级审计记录 | Pass | 见 `Week5_网络职责表.md` |
| 未修改 `.uasset` | Pass | 周一仅只读分析和文档输出 |
| 已列出 Server RPC | Pass | 重点在 `FppShooter`、`Shooter`、`ShooterPlayerState` |
| 已列出 Multicast RPC | Pass | 重点在 `FppShooter`、`Shooter`、`BP_MyGameState` |
| 已列出 RepNotify/状态变量 | Pass | `Health`、`CreatedWeapon`、`killNum`、`killedNum`、`RemainingTime`、`gameOver` |
| 已列出需编辑器确认项 | Pass | Multicast 内是否包含真实逻辑需打开蓝图确认 |

## 3. Authority Regression Cases

| Case ID | Asset | Scenario | Expected Authority Behavior | Evidence Source | Manual Verification Step | Result |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| W5-T001 | FppShooter | A 按下开火 | `server-onShootButtonDown` 处理能否开火、扣弹、命中、伤害 | `FppShooter.json:20561` | 独立服务器 + A/B，A 射击 B，看服务器日志/蓝图断点是否先进入 Server | Not Run |
| W5-T002 | FppShooter | A 开火表现 | `all-onShootButtonDown` 只播放枪声、动画、枪口火焰、后坐力 | `FppShooter.json:7901` | 打开 Multicast 节点，确认无扣弹、LineTrace、扣血 | Not Run |
| W5-T003 | FppShooter | A 松开开火 | `server-onShootButtonUp` 处理服务器认可的停火状态 | `FppShooter.json:20636` | 连发/停止测试，看 B 不出现客户端自算射击状态 | Not Run |
| W5-T004 | FppShooter | A 换弹 | Server 处理真实弹药，Multicast 只播换弹表现 | `FppShooter.json:20411`、`:7751` | A 打空弹匣后换弹，A/B 弹药显示不打架 | Not Run |
| W5-T005 | Shooter | B 被 A 击中 | 只有服务器修改 `Health` | `Shooter.json Health` 多处命中 | 在客户端尝试触发本地 Health 修改应不影响服务器真值 | Not Run |
| W5-T006 | Shooter | B 死亡 | 服务器决定死亡，`all-Die` 只播死亡表现 | `Shooter.json:9299` | A 击杀 B，检查 Health/死亡状态是否由 Server 设置 | Not Run |
| W5-T007 | ShooterPlayerState | 击杀统计 | 服务器调用 `addKillNum` 和 `addKilledNum` | `ShooterPlayerState.json:3346`、`:3228` | A 连续击杀 B 3 次，统计不丢不重复 | Not Run |
| W5-T008 | Shooter | B 复活 | `Server_Resurrect` 设置真实状态，`multicast_Resurrect` 只播表现 | `Shooter.json:22786`、`:21225` | B 复活后 A/B 看到 Health、位置、碰撞一致 | Not Run |
| W5-T009 | Shooter | A 拾枪 | 服务器决定 `CreatedWeapon` / 当前武器 / 归属 | `Shooter.json:22636`、`CreatedWeapon` | A 拾枪，B 看到同一把武器属于 A | Not Run |
| W5-T010 | Shooter | A 丢枪 | 服务器清空持有者并设置掉落状态，Multicast 只表现 | `Shooter.json:11539`、`FppShooter.json:7451` | A 丢枪，B 看到同一把武器落地 | Not Run |
| W5-T011 | Shooter | B 再拾取 A 丢下的武器 | 服务器更新唯一归属，不允许双持同一把 | `Shooter.json PickWeapon` | B 拾取后 A/B 均看到归属变化 | Not Run |
| W5-T012 | BP_MyGamemode/BP_MyGameState | 倒计时推进 | GameMode 推进，GameState 复制 `RemainingTime` | `BP_MyGameState.json:241` | 两客户端 UI 显示同一个倒计时 | Not Run |
| W5-T013 | BP_MyGameState | 比赛结束 | 服务器设置 `gameOver`，`OnRep_gameOver` 显示结束 UI | `BP_MyGameState.json:151`、`:1929` | 时间归零后两客户端同时进入结束状态 | Not Run |

## 4. Multicast Regression Cases

| Case ID | Asset | Multicast | Expected Multicast Content | Must Not Contain | Verification Step | Result |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| W5-M001 | FppShooter | `all-onShootButtonDown` | 枪声、枪口火焰、动画、后坐力表现 | 扣弹、LineTrace、ApplyDamage、Health 修改 | 打开蓝图逐节点标记 | Not Run |
| W5-M002 | FppShooter | `all-onShootButtonUp` | 停止射击表现 | 改真实开火状态、弹药 | 打开蓝图逐节点标记 | Not Run |
| W5-M003 | FppShooter | `all-onReloadButtonDown/Up` | 换弹动画、声音 | 补弹、扣弹、决定换弹完成 | 打开蓝图逐节点标记 | Not Run |
| W5-M004 | FppShooter | `all-switchWeapon` | 切枪动画、武器显示隐藏 | 改 `CurrentWeaponindex`、决定当前武器 | 打开蓝图逐节点标记 | Not Run |
| W5-M005 | FppShooter | `all-dropWeapon` | 丢枪表现、显示隐藏 | 分离真实武器、改持有者、改掉落位置真值 | 打开蓝图逐节点标记 | Not Run |
| W5-M006 | Shooter | `all-Die` | 死亡动画、音效、模型表现 | Health 修改、KillNum/KilledNum 修改、死亡判定 | 打开蓝图逐节点标记 | Not Run |
| W5-M007 | Shooter | `all-pickWeapon` / `all_attachWeapon` | Attach 显示、UI 刷新 | 设置持有者、当前武器引用 | 打开蓝图逐节点标记 | Not Run |
| W5-M008 | Shooter | `multicast_Resurrect` | 复活特效/显示 | Health、位置、死亡状态、碰撞真实状态 | 打开蓝图逐节点标记 | Not Run |
| W5-M009 | ShooterPlayerState | `all-message` | 聊天/提示显示 | 改玩家状态或计分 | 发送消息后确认只影响显示 | Not Run |
| W5-M010 | BP_MyGameState | `all-refushmapcast` / `return` | UI 或流程显示 | 决定 GameOver、倒计时、Session 权威结果 | 打开蓝图逐节点标记 | Not Run |

## 5. RepNotify And State Sync Cases

| Case ID | Asset | State | Expected Behavior | Evidence Source | Verification Step | Result |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| W5-R001 | Shooter | `Health` | 服务器修改后两个客户端一致 | `Shooter.json` 命中 `Health` | A 射击 B，A/B UI 或日志一致 | Not Run |
| W5-R002 | Shooter | `CreatedWeapon` | 服务器设置后客户端通过 OnRep 更新表现 | `Shooter.json:207`、`:7669` | 拾枪/换枪时观察第三人称武器显示 | Not Run |
| W5-R003 | Shooter | `CurrentWeaponindex` | 只由服务器切枪逻辑修改 | `Shooter.json:212` | A 快速切枪，B 看到一致结果 | Not Run |
| W5-R004 | ShooterPlayerState | `killNum` | 服务器击杀结算 +1 并同步 | `ShooterPlayerState.json:199/200` | 连续击杀后排行榜/文本一致 | Not Run |
| W5-R005 | ShooterPlayerState | `killedNum` | 服务器死亡结算 +1 并同步 | `ShooterPlayerState.json:199/200` | 连续死亡后排行榜/文本一致 | Not Run |
| W5-R006 | ShooterPlayerState | `name` / `side` / `color` | 客户端只读公开玩家状态 | `ShooterPlayerState.json` 命中变量名 | 加入游戏后双方玩家信息一致 | Not Run |
| W5-R007 | BP_MyGameState | `RemainingTime` | GameMode 推进，GameState 复制给 UI | `BP_MyGameState.json:241` | 两客户端倒计时一致 | Not Run |
| W5-R008 | BP_MyGameState | `gameOver` / `OnRep_gameOver` | 服务器设置，客户端 OnRep 显示结束 | `BP_MyGameState.json:151`、`:1929` | 时间归零后两客户端显示结束 UI | Not Run |

## 6. Evidence Completeness Check

| 资产 | 是否覆盖 | 证据摘要 | 后续补充 |
| :--- | :--- | :--- | :--- |
| FppShooter | Yes | Server/all RPC 成对；射击、瞄准、换弹、丢枪、隐藏、切枪 | 编辑器确认 all-* 内具体节点 |
| Shooter | Yes | `Health`、`CreatedWeapon`、`CurrentWeaponindex`、死亡、拾枪、复活 | 编辑器确认普通函数是否只有服务器调用 |
| Weapon | Yes | 未见明确 Net RPC / 复制属性；记录为普通武器逻辑风险 | 编辑器确认 Actor Replicates、弹药变量、持有者变量 |
| ShooterPlayerState | Yes | `killNum`、`killedNum`、`addKillNum`、`addKilledNum`、消息 RPC | 编辑器确认计分函数调用源 |
| BP_MyGamemode | Yes | GameMode 引用倒计时、GameOver、side 等规则状态 | 编辑器确认只在服务器执行 |
| BP_MyGameState | Yes | `RemainingTime`、`gameOver`、`OnRep_gameOver`、Multicast 刷新/返回 | 编辑器确认变量复制设置和 return 含义 |

## 7. Result Summary

当前状态：周一审计文档已建立，实际蓝图修改与两客户端测试尚未执行。

下一步建议：

1. 打开 UE 编辑器，从 `FppShooter.server-onShootButtonDown` 与 `FppShooter.all-onShootButtonDown` 开始核对节点。
2. 对照 `Week5_网络职责表.md` 的 W5M-001 到 W5M-012，完成射击链路标记。
3. 改动前先截图或备份蓝图，改动后按本文件 W5-T001 到 W5-T004 执行测试。
4. 每个测试失败项记录复现步骤、实际结果、预期结果和下一步修复优先级。
