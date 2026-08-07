# Week5 网络职责表

生成时间：2026-05-26

## 1. Scope And Source Evidence

本文件是第 5 周周一“蓝图网络审计与标记”的只读产出。分析对象为 UE4.27 项目 `project10` 中当前重点的 6 个蓝图资产导出的 UAssetAPI JSON：

| 资产 | JSON |
| :--- | :--- |
| FppShooter | `C:\Users\19370\.local\share\opencode\uasset-json\project10\FppShooter.json` |
| Shooter | `C:\Users\19370\.local\share\opencode\uasset-json\project10\Shooter.json` |
| Weapon | `C:\Users\19370\.local\share\opencode\uasset-json\project10\Weapon.json` |
| ShooterPlayerState | `C:\Users\19370\.local\share\opencode\uasset-json\project10\ShooterPlayerState.json` |
| BP_MyGamemode | `C:\Users\19370\.local\share\opencode\uasset-json\project10\BP_MyGamemode.json` |
| BP_MyGameState | `C:\Users\19370\.local\share\opencode\uasset-json\project10\BP_MyGameState.json` |

本次没有修改任何 `.uasset`、`.umap` 或 C++ 文件。UAssetAPI JSON 是低层序列化结构，不等同于 UE 编辑器中的完整蓝图图表；本文只记录能从函数名、`FunctionFlags`、变量名、`VirtualFunctionName`、`OnRep_`、`RepNotify` 等证据支持的结论。无法确认的节点标记为“需编辑器确认”。

## 2. Authority Classification Summary

| 资产 | 当前职责 | 已见 Server RPC | 已见 Multicast RPC | 已见复制/RepNotify 状态 | 初步结论 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| FppShooter | 输入与射击/瞄准/换弹/切枪 RPC 转发 | `server-onShootButtonDown`、`server-onShootButtonUp`、`Server-onAimButtonDown`、`Server-onAimButtonUp`、`Server-onReloadButtonDown`、`Server-onReloadButtonUp`、`Server-dropWeapon`、`server-hideweapon`、`Server-switchWeapon` | `all-onShootButtonDown`、`all-onShootButtonUp`、`all-onAimButtonDown`、`all-onAimButtonUp`、`all-onReloadButtonDown`、`all-onReloadButtonUp`、`all-dropWeapon`、`all-hideweapon`、`all-switchWeapon` | 命中 `Health`、`getCurrentWeapon` 引用 | Server/Multicast 成对存在，下一步重点确认 all-* 是否只做表现 |
| Shooter | 角色真实状态、武器持有、死亡复活 | `Server_Resurrect` | `all-Die`、`all-pickWeapon`、`all_attachWeapon`、`multicast_Resurrect` | `Health`、`CreatedWeapon`、`CurrentWeaponindex`、`OnRep_CreatedWeapon` | 风险最高，Health、死亡、拾枪、复活必须服务器权威 |
| Weapon | 武器普通逻辑、数据表、Mesh、特效、射击方向 | 未发现明确 NetServer FunctionExport | 未发现明确 NetMulticast FunctionExport | `NumReplicatedProperties` 显示 0；未见明确 RepNotify | 需要确认是否由服务器路径调用，不能让客户端直接决定伤害/弹药/归属 |
| ShooterPlayerState | 玩家公开状态、击杀/死亡数、消息 | `Server-message` | `all-message` | `killNum`、`killedNum`、`name`、`side`、`color` | `addKillNum/addKilledNum` 不是 Server RPC，必须只允许服务器调用 |
| BP_MyGamemode | 服务器规则中心 | 未发现 Net RPC | 未发现 Net RPC | 引用 `RemainingTime`、`gameOver`、`side` | 符合 GameMode 服务器规则定位，继续保持客户端不直接改规则 |
| BP_MyGameState | 公开比赛状态 | 未发现 Server RPC | `all-refushmapcast`、`return` | `RemainingTime`、`gameOver`、`OnRep_gameOver` | `gameOver` 已有 OnRep 迹象，`return` 命名需确认不承担真实规则 |

## 3. Multicast True Logic Findings

以下是周一需要在 UE 编辑器中重点打开确认的 Multicast 或类似广播函数。JSON 证据能定位函数存在和相关变量/调用，但不能完全证明节点连线语义。

| 风险级别 | 资产 | Multicast / 广播函数 | 为什么需要确认 | 应保留内容 | 不应包含内容 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| 高 | FppShooter | `all-onShootButtonDown` | 射击表现链路，可能与 `OnShootButtonDown`、`getCurrentWeapon` 相关 | 枪声、枪口火焰、射击动画、后坐力表现 | 扣子弹、LineTrace、扣血、死亡判定 |
| 高 | FppShooter | `all-onShootButtonUp` | 射击停止链路，可能影响武器状态 | 停止射击动画/音效 | 改真实开火状态、改弹药 |
| 高 | FppShooter | `all-switchWeapon` | 带 `index` 参数，疑似同步切枪 | 切枪动画、显示隐藏 | 改当前武器权威索引、决定武器归属 |
| 高 | FppShooter | `all-dropWeapon` | 丢枪可能涉及真实武器归属 | 丢枪表现、显示隐藏 | 分离/归属/掉落位置真实状态 |
| 中 | FppShooter | `all-hideweapon` | 隐藏武器可能只是表现，也可能销毁/隐藏真实武器 | 第一/第三人称显示隐藏 | 销毁真实武器 Actor 或改拥有者 |
| 中 | FppShooter | `all-onReloadButtonDown/Up` | 换弹可能涉及弹药 | 换弹动画、声音 | 扣/补真实弹药 |
| 低 | FppShooter | `all-onAimButtonDown/Up` | 开镜通常本地表现，但可能影响散布/移动 | 开镜视觉、镜头/动画 | 改服务器命中参数，除非由 Server 决定 |
| 高 | Shooter | `all-Die` | 死亡是核心真实状态 | 死亡动画、音效、模型表现 | `Health=0`、禁用输入/碰撞、击杀统计 |
| 高 | Shooter | `all-pickWeapon` | 拾枪涉及归属 | Attach 表现、UI 刷新 | 设置持有者、当前武器引用、隐藏地面武器 |
| 高 | Shooter | `all_attachWeapon` | Attach 名称直接涉及武器表现/归属边界 | 武器挂点表现 | 决定真实归属或当前武器 |
| 高 | Shooter | `multicast_Resurrect` | 复活涉及 Health、位置、碰撞、输入 | 复活特效/显示 | `Health=Max`、死亡状态 false、传送位置 |
| 低 | ShooterPlayerState | `all-message` | 消息广播通常不是玩法真相 | 聊天/提示显示 | 改击杀数、改玩家状态 |
| 中 | BP_MyGameState | `all-refushmapcast` | 命名像刷新地图/玩家列表 | UI 刷新 | 改比赛状态、改玩家分组 |
| 中 | BP_MyGameState | `return` | 名字不清晰，且是 Multicast | 返回菜单显示/流程提示 | 决定 GameOver、销毁 Session 的权威结果 |

## 4. Node/Function-Level Audit Records

| ID | Asset | Node/Function | Type | Authority Role | Evidence | Game Logic Present | Risk | Monday Recommendation |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| W5M-001 | FppShooter | `server-onShootButtonDown` | Server RPC | 客户端请求服务器开火 | `FppShooter.json:20561 ObjectName`，`FunctionFlags FUNC_NetServer` 附近 | 是，射击入口 | 高 | 打开蓝图确认能否开火、扣弹、命中、伤害是否在这里 |
| W5M-002 | FppShooter | `all-onShootButtonDown` | NetMulticast | 开火表现广播 | `FppShooter.json:7901 ObjectName`，`FunctionFlags FUNC_NetMulticast` 附近 | 需确认 | 高 | 只保留枪声、动画、枪口火焰、后坐力表现 |
| W5M-003 | FppShooter | `server-onShootButtonUp` | Server RPC | 客户端请求停止开火 | `FppShooter.json:20636 ObjectName` | 是，射击停止入口 | 中 | 确认只改服务器认可的开火状态，不在客户端自算 |
| W5M-004 | FppShooter | `all-onShootButtonUp` | NetMulticast | 停火表现广播 | `FppShooter.json:7976 ObjectName` | 需确认 | 中 | 只保留动画/音效停止表现 |
| W5M-005 | FppShooter | `Server-onReloadButtonDown` | Server RPC | 换弹请求入口 | `FppShooter.json:20411 ObjectName` | 可能涉及弹药 | 高 | 真实弹药变化必须在 Server，Multicast 只播换弹表现 |
| W5M-006 | FppShooter | `all-onReloadButtonDown` | NetMulticast | 换弹表现广播 | `FppShooter.json:7751 ObjectName` | 需确认 | 高 | 不得补弹、扣弹或决定换弹完成 |
| W5M-007 | FppShooter | `Server-switchWeapon` | Server RPC | 切枪请求入口 | `FppShooter.json:20758 ObjectName` | 是，武器索引/当前武器 | 高 | 服务器验证 index、武器存在、未死亡后改真实当前武器 |
| W5M-008 | FppShooter | `all-switchWeapon` | NetMulticast | 切枪表现广播 | `FppShooter.json:8098 ObjectName`，参数 `index` | 可能涉及真实当前武器 | 高 | 不得决定 `CurrentWeaponindex`，只做显示/动画 |
| W5M-009 | FppShooter | `Server-dropWeapon` | Server RPC | 丢枪请求入口 | `FppShooter.json:20111 ObjectName` | 是，武器归属 | 高 | 服务器清空持有者、分离、设置掉落位置 |
| W5M-010 | FppShooter | `all-dropWeapon` | NetMulticast | 丢枪表现广播 | `FppShooter.json:7451 ObjectName` | 可能涉及武器归属 | 高 | 只保留视觉/音效/显示隐藏修正 |
| W5M-011 | FppShooter | `Server-onAimButtonDown/Up` | Server RPC | 瞄准请求或同步入口 | `FppShooter.json:20261`、`20336 ObjectName` | 可能影响散布/移动 | 中 | 如果影响命中参数，服务器决定；纯开镜视觉可本地 |
| W5M-012 | FppShooter | `all-onAimButtonDown/Up` | NetMulticast | 瞄准表现广播 | `FppShooter.json:7601`、`7676 ObjectName` | 多半表现 | 低 | 保留相机/动画/UI 表现，真实命中参数不要放这里 |
| W5M-013 | Shooter | `Health` | Replicated/状态变量候选 | 角色生命值真相 | `Shooter.json:338 Name`，多处 `Value Health` | 是 | 高 | 所有 Health 修改应在服务器伤害/复活链路 |
| W5M-014 | Shooter | `all-Die` | NetMulticast | 死亡表现广播 | `Shooter.json:9299 ObjectName`，参数 `Enemy` | 可能混入死亡真相 | 高 | 不得改 Health、死亡状态、KillNum/KilledNum |
| W5M-015 | Shooter | `Server_Resurrect` | Server RPC | 复活真实状态入口 | `Shooter.json:22786 ObjectName` | 是 | 高 | 设置 Health、死亡状态、碰撞/输入、位置、默认武器 |
| W5M-016 | Shooter | `multicast_Resurrect` | NetMulticast | 复活表现广播 | `Shooter.json:21225 ObjectName` | 可能混入真实复活 | 高 | 只播复活表现，不改真实 Health/位置 |
| W5M-017 | Shooter | `CreatedWeapon` | RepNotify 状态 | 已创建/当前武器引用相关 | `Shooter.json:207 Name`，`OnRep_CreatedWeapon` | 是 | 高 | 服务器设置引用，客户端通过 OnRep 更新表现 |
| W5M-018 | Shooter | `OnRep_CreatedWeapon` | RepNotify | 武器引用变化后的客户端响应 | `Shooter.json:7669`、`21773 ObjectName` | 表现/同步响应 | 中 | 只做显示、Mesh、UI 响应，不决定归属 |
| W5M-019 | Shooter | `CurrentWeaponindex` | 状态变量候选 | 当前武器索引 | `Shooter.json:212 Name`，多处 `Value CurrentWeaponindex` | 是 | 高 | 只由服务器切枪/拾枪/丢枪逻辑修改 |
| W5M-020 | Shooter | `PickWeapon` | 普通函数 | 拾枪逻辑 | `Shooter.json:7993`、`22636 ObjectName` | 是 | 高 | 确认只从服务器路径调用，或改为 Server RPC/Authority 分支 |
| W5M-021 | Shooter | `all-pickWeapon` | NetMulticast | 拾枪表现广播 | `Shooter.json:9422 ObjectName`，参数 `Weapon` | 可能涉及归属 | 高 | 不得设置持有者/当前武器，只表现 Attach/UI |
| W5M-022 | Shooter | `DropWeapon` | 普通函数 | 丢枪逻辑 | `Shooter.json:4081`、`11539 ObjectName` | 是 | 高 | 确认 Authority 后执行，客户端只请求 |
| W5M-023 | Shooter | `setCurrentWeapon` | 普通函数 | 设置当前武器 | `Shooter.json:8122`、`23016 ObjectName` | 是 | 高 | 只允许服务器调用，客户端表现走 OnRep/Multicast |
| W5M-024 | ShooterPlayerState | `addKillNum` | 普通函数 | 击杀数 +1 | `ShooterPlayerState.json:3346`、`4655 ObjectName`，`killNum` 多处命中 | 是 | 高 | 只能由服务器死亡结算调用 |
| W5M-025 | ShooterPlayerState | `addKilledNum` | 普通函数 | 死亡数 +1 | `ShooterPlayerState.json:3228`、`4392 ObjectName`，`killedNum` 多处命中 | 是 | 高 | 只能由服务器死亡结算调用 |
| W5M-026 | ShooterPlayerState | `Server-message` | Server RPC | 消息请求入口 | `ShooterPlayerState.json:7501 ObjectName` | 非核心玩法 | 低 | 保持消息和击杀统计分离 |
| W5M-027 | ShooterPlayerState | `all-message` | NetMulticast | 消息显示广播 | `ShooterPlayerState.json:4777 ObjectName` | UI/消息 | 低 | 不得混入玩家状态修改 |
| W5M-028 | BP_MyGamemode | `ExecuteUbergraph_BP_MyGamemode` | GameMode 规则图 | 服务器规则、倒计时、GameState 写入 | `BP_MyGamemode.json:3600 FunctionFlags`，命中 `RemainingTime`、`gameOver` | 是 | 高 | 保持规则只在服务器执行，客户端不直接改 GameState 真相 |
| W5M-029 | BP_MyGameState | `RemainingTime` | 复制状态候选 | 客户端显示倒计时 | `BP_MyGameState.json:241 Name` | 是 | 高 | 确认 Replicated，GameMode 推进，UI 只读 |
| W5M-030 | BP_MyGameState | `gameOver` / `OnRep_gameOver` | RepNotify 状态 | 比赛结束公开状态 | `BP_MyGameState.json:151 gameOver`、`1929 OnRep_gameOver` | 是 | 高 | GameMode 设置，GameState 复制，OnRep 只显示结束 UI |
| W5M-031 | BP_MyGameState | `all-refushmapcast` | NetMulticast | 刷新地图/玩家列表广播 | `BP_MyGameState.json:2111 ObjectName` | 需确认 | 中 | 只做 UI/地图刷新，不改比赛规则 |
| W5M-032 | BP_MyGameState | `return` | NetMulticast | 返回流程广播 | `BP_MyGameState.json:4189 ObjectName` | 需确认 | 中 | 不承担 GameOver 判定或 Session 权威销毁 |
| W5M-033 | Weapon | `Weapon_C` asset registry | 普通 Actor 蓝图 | 武器基础类 | `Weapon.json` 显示 `NumReplicatedProperties: 0`，未发现明确 Net RPC | 可能有普通射击/特效逻辑 | 中 | 检查是否 `Replicates`、是否只被服务器真实逻辑调用 |

## 5. Asset-by-Asset Notes

### FppShooter

`FppShooter` 当前最像输入层和 RPC 转发层。Server 与 all-* Multicast 基本成对，说明结构已经接近“客户端请求 Server，Server 广播表现”。周二应优先打开 `server-onShootButtonDown` 和 `all-onShootButtonDown`，确认扣弹、LineTrace、伤害是否在 Server 内。

### Shooter

`Shooter` 是真实状态聚合点，已见 `Health`、`CreatedWeapon`、`CurrentWeaponindex`、死亡、拾枪、复活相关函数。周一标记结论是：这里是第 5 周最重要的重构对象，任何 Health、死亡、复活、武器归属修改都应归到 Server/Authority。

### Weapon

`Weapon` 未见明确网络 RPC 和复制属性。它可能是普通 Actor 蓝图，承担 Mesh、数据表、射击方向、声音、Decal、特效等逻辑。后续应确认武器 Actor 是否 `Replicates`，以及弹药/持有者是否散落在普通函数中。

### ShooterPlayerState

`ShooterPlayerState` 包含 `killNum`、`killedNum`、`name`、`side`、`color`。`addKillNum` / `addKilledNum` 不是 Server RPC，因此必须从服务器死亡结算链路调用。`Server-message` / `all-message` 可保留为消息广播，但不要混入计分。

### BP_MyGamemode

`BP_MyGamemode` 没有发现 Net RPC，符合 GameMode 只在服务器存在的规则中心定位。它应负责倒计时推进、GameOver 判定、复活规则和写入 GameState。

### BP_MyGameState

`BP_MyGameState` 已见 `RemainingTime`、`gameOver`、`OnRep_gameOver`，适合作为客户端 UI 读取的公开比赛状态。`all-refushmapcast` 和 `return` 需要在编辑器中确认是否只是表现/流程通知。

## 6. Monday Risk List

1. `FppShooter.all-onShootButtonDown` 可能仍承担扣弹、命中、伤害等真实逻辑。
2. `FppShooter.all-switchWeapon` 和 `all-dropWeapon` 带武器相关语义，可能混入真实归属修改。
3. `Shooter.all-Die` 可能混入 Health、死亡状态、击杀统计修改。
4. `Shooter.multicast_Resurrect` 可能混入真实复活状态修改。
5. `Shooter.PickWeapon`、`DropWeapon`、`setCurrentWeapon` 是普通函数，必须确认只由服务器调用。
6. `ShooterPlayerState.addKillNum/addKilledNum` 是普通函数，必须确认没有客户端调用路径。
7. `BP_MyGameState.return` 命名不清，需确认不是用 Multicast 决定比赛结束。
8. `Weapon` 暂未见复制属性，若地面武器需要所有客户端一致，可能需要启用 Actor 复制或改由服务器生成/同步。

## 7. Deferred Work, Not In Scope

本周一不做以下事项：

- 不修改 `.uasset`。
- 不新增护甲、经济、比分、炸弹系统。
- 不开始全量 C++ 迁移。
- 不重写 UI、动画、地图、美术资源。
- 不处理公网部署、Master Server、Linux Server。
- 不把 `Desktop\cache` 当长期存储。

下一步进入周二前，建议在 UE 编辑器中按本表从 `FppShooter.server-onShootButtonDown` 与 `FppShooter.all-onShootButtonDown` 开始逐节点核对。
