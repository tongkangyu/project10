# 多人游戏架构契约

## 目的

本文定义 UE5.8 技术演示路线中当前的多人游戏状态归属契约。它记录现有的蓝图优先架构和 C++ 权威化目标，但不将 Dedicated Server 兼容性或服务端权威战斗标记为已验证完成。

## 基线

| 项目 | 值 |
| :--- | :--- |
| 项目名 | `project09` |
| 引擎关联版本 | UE5.8 |
| 已推送稳定提交 | `6d71619` |
| 默认 GameMode | `/Game/All/BP_MyGamemode.BP_MyGamemode_C` |
| 默认地图 | `/Game/Season7/Main.Main` |
| Dedicated Server 验证地图 | `/Game/Maps/csgo` |
| 当前开发平台 | Windows |
| 后续公网服务器 | 腾讯云固定公网 IP |
| Linux 适配 | Windows 网络逻辑稳定后最后处理 |

`/Game/Maps/csgo` 与当前默认地图不同。M1 的启动命令必须显式指定 `csgo`；本次文档工作不修改 `DefaultEngine.ini`。

## 当前类链

```text
BP_MyGameInstance
  -> UMyGameInstance_CPP

BP_MyGamemode
  -> ARoomGameMode
     -> AGameModeBase

BP_MyGameState
  -> ARoomGameState
     -> AGameStateBase

BP_MyPlayerController
  -> ARoomPlayerController
     -> APlayerController

ShooterPlayerState
  -> APlayerState

FppShooter
  -> Shooter
     -> Character
        -> Pawn

Weapon
  -> Actor
```

`UMyGameInstance_CPP` 当前负责 Listen Server 跳转、客户端跳转和网络失败事件处理。它是会话入口工具，而不是游戏玩法权威层。

## 状态归属契约

| 范围 | Client | Server | 复制结果 |
| :--- | :--- | :--- | :--- |
| 输入 | 收集移动与战斗意图并提交请求 | 校验并接受或拒绝请求 | 仅复制已接受的状态 |
| 比赛规则 | 读取并显示状态 | 队伍分配、出生规则、计时、比赛结束、重置、跳图 | 通过 GameState 复制比赛状态与计时 |
| 战斗 | 仅预测或播放本地表现 | 校验射速、装备武器、弹药、射线、目标资格、伤害、死亡与得分 | 生命、弹药、死亡结果、得分、当前武器 |
| 武器 | 请求装备、丢弃、拾取；根据状态更新表现 | 校验归属与状态切换；创建、挂接、分离并配置掉落武器 | Owner、当前武器、掉落 Actor 状态 |
| 玩家公开信息 | 读取公开玩家信息 | 维护公开玩家数据的唯一真相 | 通过 PlayerState 复制名字、队伍、击杀、死亡 |
| 表现 | UI、动画、粒子、音效、视觉后坐力 | 在动作被接受后触发表现结果 | Cosmetic RPC 不建立玩法真相 |

客户端绝不能权威性地写入生命、弹药、得分、队伍、武器归属或比赛状态。Multicast RPC 是服务器已接受状态的表现结果，不能成为状态来源。

## 当前职责

| 类或资产 | 当前职责 | 必须遵守的权威边界 |
| :--- | :--- | :--- |
| `BP_MyGamemode` | 登录/登出、队伍计数、出生点选择、小地图刷新；当前保留 `StartRound` 触发 | 仅服务端规则所有者；客户端不写入规则状态 |
| `ARoomGameMode` | 回合倒计时、空房计时、回合结束、Session 返回流程的服务端部分 | 只在服务器修改回合状态和房间计时器 |
| `BP_MyGameState` | 接收 `RoundState`，映射 `RemainingTime`，显示 `gameOver` UI | 接收服务器写入的状态，并复制到客户端 |
| `ARoomGameState` | 复制 `RoundState` 和回合阶段 | `RoundState` 是回合时间和阶段的真实来源 |
| `BP_MyPlayerController` | 本地玩家控制、UI 初始化和退出清理 | 不得使用未复制的 Controller 字段作为远端可见的名字或队伍真相 |
| `ARoomPlayerController` | 回合结束后的 Session 清理和返回主菜单 | 客户端只处理自己的本地跳转，不决定服务器回合结果 |
| `ShooterPlayerState` | 公开名字、阵营、击杀/死亡数、复活相关数据 | 远端可见身份与得分的唯一来源 |
| `FppShooter` | 输入以及 Server/Multicast RPC 转发 | 客户端请求必须进入服务端校验；Multicast 只能做表现 |
| `Shooter` | 生命、死亡、当前武器、装备、拾取、丢弃、复活 | 所有玩法状态和武器归属的变更均由服务器负责 |
| `Weapon` | 武器数据、射击支持、Mesh、特效、掉落/装备表现 | 服务器负责弹药、伤害、Owner、挂接和掉落状态 |

## 目标战斗流程

```text
客户端开火请求
  -> 服务器校验装备武器、归属、射速、弹药、射线、射程和目标资格
  -> 服务器消耗弹药并施加伤害
  -> 服务器结算死亡、得分和武器掉落状态
  -> 复制状态到达所有相关客户端
  -> 本地或 Multicast 表现播放已接受动作的反馈

客户端装备/丢弃/拾取请求
  -> 服务器校验 Pawn 状态、槽位、归属和 Actor 可用性
  -> 服务器原子性更新 Owner、挂接、物理和复制引用
  -> 客户端根据复制状态更新表现
```

## 证据与待验证项

保留以下现有蓝图审计文档作为证据，不替代或改写它们：

- `Docs/Week5_网络职责表.md`
- `Docs/Week5_回归测试记录.md`

在开始 C++ 战斗切片前，必须在编辑器中确认 `all-*` Multicast 函数不会修改弹药、生命、得分、武器归属或比赛规则。尤其要确认 Week5 审计中指出的开火、换弹、切枪、丢枪、死亡、拾取和复活链路。

## 当前运行方向

```text
本机 Windows 开发和验证
  -> Windows Dedicated Server 打包
  -> 腾讯云固定公网 IP 验证
  -> 服务器端权威和测试级反作弊
  -> 最后 Linux Dedicated Server 适配
```

当前不使用：

```text
EOS
Steam
账号登录
普通玩家服务器密码
服务器列表
自动封禁
```

## 里程碑边界

M1 用于在 UE5.8 Windows 环境中完成现有房间生命周期的 C++ 迁移。M1 不进行大范围战斗玩法重构，也不处理 Linux。

M2 用于在 Windows Dedicated Server 上打包和验证当前房间逻辑。

M3 引入服务端权威战斗切片的 C++ 基类，同时保留蓝图子类承载资产、动画、UI 和地图表现。预期引入顺序为 `AShooterGameState`、`AShooterPlayerState`、`AShooterCharacterBase`、`AWeaponBase`、`AShooterGameMode` 和 `AShooterPlayerController`。
