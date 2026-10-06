# 代码来源与依赖

本工程集成参考实现、商家 SDK 和开源主站，不是完整 EtherCAT 协议栈的自主实现。

| 部分 | 来源及版本 | 仓库中如何保存 |
|---|---|---|
| 从站协议栈 | 商家提供的 Beckhoff SSC 5.11 | 原 SDK 不提交，使用准备脚本从用户合法取得的原 ZIP 提取并应用适配 |
| STM32 驱动基础 | 商家工程中的 ST 标准外设库及芯片基础文件 | 随原 SDK 本地恢复，保留原版权与许可说明 |
| 主站 | OpenEtherCATsociety/SOEM，固定提交 `88e8ed46efba7dfa7b94d08a512db25a33e3f8d5` | 本地 vendor 准备，不随本项目提交；许可证由上游保留 |
| eepromtool | SOEM 自带工具 | 从上游源码构建，不认领为自写工具 |
| 应用与工程适配 | common、ssc_bridge、主站演示、GNU 工程与工具脚本 | 参考工程及辅助工具生成后用于逐课学习、修改与验证 |

SOEM 上游：[源码仓库](https://github.com/OpenEtherCATsociety/SOEM)、[固定版本许可](https://github.com/OpenEtherCATsociety/SOEM/blob/88e8ed46efba7dfa7b94d08a512db25a33e3f8d5/LICENSE.md)。上游采用 GPLv3/商业许可方式；具体使用与分发按其许可处理。本仓库没有将第三方代码重新授权为 MIT 等其他许可。

商家 ZIP 的 SHA-256 为 `d1d9f970d62029b56df948c9af8e320c497777ab24e36f3f5341572b17c22848`，来源记录见 `config/source_manifest.json`。本项目不公开分发整份购买资料、原始 SDK 或板卡 Flash/EEPROM 备份。

学习者的个人贡献以课程练习、本人修改记录和独立复现为依据；完整参考工程存在，不表示每个模块都由学习者独立编写。
