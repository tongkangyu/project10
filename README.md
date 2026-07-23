# UE4.27 多人 FPS Dedicated Server Demo

这是一个以 **Unreal Engine 4.27** 开发的多人第一人称射击功能原型，重点验证 Windows Dedicated Server 构建、服务端权威玩法、武器同步和多人对局状态管理。

项目以蓝图实现主要 Gameplay，以 C++ 补充网络入口和独立服务器构建能力。当前稳定版本为 [`v0.8-ds-weapon-drop-fix`](https://github.com/tongkangyu/project10/releases/tag/v0.8-ds-weapon-drop-fix)，提供可直接下载的 Windows 客户端与 Dedicated Server。

> 仓库名为 `project10`，Unreal 工程文件因早期命名沿用为 `project09.uproject`。

## 快速体验

无需安装 Unreal Editor，可从 Release 下载同一版本的客户端和服务端：

- [v0.8 Windows Client / Dedicated Server 下载](https://github.com/tongkangyu/project10/releases/tag/v0.8-ds-weapon-drop-fix)
- 默认地图：`/Game/Maps/csgo`
- 默认端口：UDP `7777`

### 本地双客户端测试

1. 下载并解压 Server 与 Client 压缩包。
2. 运行服务端中的：

   ```text
   WindowsServer\project09\Binaries\Win64\start_server.bat
   ```

3. 启动第一个客户端，在主菜单 IP 输入区域连接 `127.0.0.1`。
4. 再启动一个客户端并连接同一地址。
5. 测试移动、拾枪、丢枪、切枪、射击、死亡、复活与比赛重置。

客户端与服务端必须来自同一个 Release，不能混用不同版本的 Cook 或打包产物。

## 核心内容

### 多人射击玩法

- 第一人称移动、瞄准、射击、换弹与武器切换。
- 支持 AK47、AUG、AWP、Deagle、P90、Shotgun 和 Knife 等武器资产。
- 武器拾取、持有、丢弃、死亡掉落与再次拾取。
- 生命、伤害、死亡、复活、击杀/死亡统计与比赛倒计时。
- AWP 开镜射击后自动关镜、第一人称后坐力和远端射击表现。

### 网络职责

核心玩法按照“客户端提交意图，服务器决定结果，客户端播放表现”的方向组织：

```text
客户端输入
  -> Server RPC 提交射击、瞄准、换弹、切枪或丢枪请求
  -> Dedicated Server 更新弹药、命中、伤害、生命和武器归属等状态
  -> Replication / RepNotify 同步持续状态
  -> Owning Client RPC / Multicast 播放后坐力、弹痕、动画和音效等表现
```

- 屏幕中心准星转换为世界空间射线，连射期间持续更新 `AimStart` / `AimDirection`。
- 服务器执行 LineTrace、弹药消耗、伤害与死亡结算，降低客户端直接修改玩法状态的风险。
- 第一人称后坐力仅发送给武器拥有者，其他客户端接收第三人称表现。
- 对霰弹枪远端弹痕广播进行节流，避免每颗弹丸都触发一次 Multicast。
- GameMode 负责比赛规则与空房重置，GameState / PlayerState 承载需要同步的公开状态。

### Dedicated Server 工程化

- 提供 `project09Server.Target.cs`，支持 Windows Server Target 编译。
- 使用 `RunUAT BuildCookRun` 完成 Client / Server 的 Stage、Pak 和 Archive。
- 服务端可通过 `-nullrhi -unattended` 无渲染启动，并将日志写入独立文件。
- 客户端通过 IP 和端口直连服务器，并监听 Unreal 网络失败事件。
- 已形成 Packaged Server + 两个独立 Packaged Client 的本地验证链路。

## v0.8 重点修复

[`v0.8-ds-weapon-drop-fix`](https://github.com/tongkangyu/project10/releases/tag/v0.8-ds-weapon-drop-fix) 基于提交 `8a811e7` 构建，主要处理：

- 玩家全部离开后，清理旧倒计时和房间状态，避免重连继承上一局状态。
- 防止 Dedicated Server 在比赛结束或登出流程中跳转到客户端菜单地图。
- 减少 Dedicated Server 上不应执行的 UI / Widget 行为。
- 修复武器丢弃时与玩家、墙面、地面或角落重叠，开启物理后被高速弹飞的问题。
- 在启用掉落物理前加入偏移和 Sweep 检查，改善掉落位置稳定性。
- 降低移动中丢枪后被原持有者立即重新拾取的概率。

Release 同时提供 Client / Server 压缩包、提交来源、SHA256 和回归检查项。

## 从源码运行

### 环境要求

- Windows 10 / 11
- Unreal Engine 4.27 源码版
- Visual Studio 2019，安装“使用 C++ 的游戏开发”工作负载
- Git LFS

Launcher 安装的 UE4.27 不能直接构建本项目的 Dedicated Server Target，服务端打包需要源码版引擎。

### 获取工程

```powershell
git clone --branch CPPLine https://github.com/tongkangyu/project10.git
Set-Location project10
git lfs pull
```

右键 `project09.uproject` 生成 Visual Studio 工程文件，编译 `Development Editor | Win64` 后即可使用 UE4.27 Editor 打开。

### 打包脚本

仓库根目录提供：

- `build_client.bat`：编译并归档 Windows Client。
- `build_server.bat`：编译并归档 Windows Dedicated Server，同时生成启动脚本。

两个脚本默认使用以下源码引擎路径：

```text
C:\UE4Source\UnrealEngine
```

如果本机路径不同，需要修改脚本中的 `ENGINE_ROOT`。脚本使用 `-skipcook`，运行前必须先在 Project Launcher 中分别 Cook `WindowsNoEditor` 和 `WindowsServer` 内容。

蓝图或地图修改后不要直接复用旧 Cook 结果，否则改动可能不会进入最终包体。

### 服务端手动启动

```powershell
project09Server.exe /Game/Maps/csgo -log -port=7777 -nullrhi -unattended
```

局域网测试时，客户端连接服务器局域网 IP；跨机器连接还需要放行 Windows 防火墙 UDP `7777`。

## 项目结构

```text
project10/
|-- Config/                         项目、地图与打包配置
|-- Content/                        蓝图、地图、武器、UI 与美术资源
|   |-- All/                        Gameplay 核心蓝图
|   |-- Maps/csgo.umap              Dedicated Server 验证地图
|   `-- Season7/                    菜单与其他地图资源
|-- Docs/                           网络职责审计与回归测试记录
|-- Plugins/SimpleNetworkFunctions/ 项目内网络辅助插件
|-- Source/project09/               C++ Runtime 模块与 GameInstance 网络入口
|-- Source/project09Server.Target.cs Windows Dedicated Server Target
|-- build_client.bat                客户端打包脚本
|-- build_server.bat                服务端打包脚本
`-- project09.uproject              Unreal 工程入口
```

关键蓝图职责：

| 资产 | 职责 |
| --- | --- |
| `BP_MyGamemode` | 登录/登出、队伍、出生、倒计时、比赛结束与空房重置 |
| `BP_MyGameState` | 对局倒计时和比赛公开状态 |
| `ShooterPlayerState` | 玩家名称、阵营、击杀与死亡数据 |
| `FppShooter` | 本地输入与 Server / Multicast RPC 转发 |
| `Shooter` | 生命、死亡、复活和当前武器状态 |
| `Weapon` | 武器数据、射击支持、特效与掉落表现 |

## 当前边界

- 稳定版本为 UE4.27 Windows Client + Windows Dedicated Server。
- 当前连接方式为 IP 直连，没有实现 Master Server 或完整的公网服务器发现服务。
- 尚未完成 Linux Dedicated Server、公网部署、压力测试以及高延迟/丢包环境测试。
- Gameplay 仍以蓝图为主，并非全量 C++ 项目。
- UE5.8 迁移工作位于 [`CPPLine-5.8`](https://github.com/tongkangyu/project10/tree/CPPLine-5.8)，不代表当前稳定 Release 已升级至 UE5.8。

## License

本人编写的代码与文档采用 [MIT License](LICENSE)。`Content/` 中可能包含的第三方模型、动画、音频、贴图及其他素材不属于 MIT 授权范围，其权利归原作者所有；本仓库仅用于学习和技术展示。
