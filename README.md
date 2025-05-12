# QQ 即时通讯应用（基于 Qt） 💬


[![GitHub Stars](https://img.shields.io/github/stars/692leaf/QQ?style=flat-square)](https://github.com/692leaf/QQ/stargazers)
[![License](https://img.shields.io/badge/License-MIT-blue.svg?style=flat-square)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows-0078d7.svg?style=flat-square)](https://www.microsoft.com/windows)

> 基于 Qt 框架开发的类 QQ 即时通讯应用，支持富文本聊天、文件传输、视频通话等功能，专为 Windows 系统设计。

---

## ✨ 核心功能

### **聊天功能**
- **富文本消息**：支持文字、表情包、图片混合编辑（类 Word 排版）
- **气泡式聊天窗口**：仿 QQ 风格的消息气泡界面

### **文件传输**
- **分块传输**：大文件自动分块上传/下载（不支持断点续传）
- **历史记录**：自动保存最近传输文件列表

### **音视频通话**
- 🎥 视频通话：基于 Qt 的摄像头捕获
- 🎙️ 语音通话：实时音频传输


### **其他特性**
- 托盘驻留：最小化到系统托盘
- 多窗口管理：独立聊天窗口与主界面分离

---

## 🔧 待实现功能（开发路线图）
### **界面优化**
- **主题切换**：浅色/深色模式（基于 Qt 样式表）
- **进度显示**：文件传输实时速度/百分比进度条

### **功能增强**
- **通话管理**：呼叫等待/挂断/最小化窗口控制
- **断点续传**：支持传输中断后恢复任务
- **消息分页**：数据库分页加载聊天记录

扩展模块
- **QQ 空间网页**：基于 Vue3 + Element Plus 的社交功能扩展
- **事务安全**：数据库事务处理保障数据一致性


## 🛠️ 环境要求

| 组件          | 版本要求               |
|---------------|---------------------- |
| 操作系统      | Windows 10/11 (x64)   |
| Qt 框架       | 6.5.3                 |
| 编译器        | MSVC 2019 / MinGW     |
| 数据库        | mysql  9.2            |

---

## 🚀 快速部署

### 1. 获取代码
```
git clone https://github.com/692leaf/QQ.git
```

### 2. 加载依赖
```
由于 Qt 默认不包含 MySQL 驱动，需手动编译：
1.编译\Qt\"版本号"\Src\qtbase\src\plugins\sqldrivers\CMakeLists.txt
2.将\Qt\"版本号"\Src\qtbase\src\plugins\sqldrivers\build\Desktop_Qt_版本号_MinGW/MSVC_64_bit-Debug\plugins\sqldrivers\目录下与mysql有关的文件拷贝到MinGW/MSVC_64的bin目录下面

```

### 3. 速览代码结构
```

🗂️ 项目结构
QQ/
QQ/
├── client/ # 客户端源码
│ ├── common/            # 公共组件
│ ├── loginInterface/    # 登录模块
│ └── mainInterface/     # 主界面模块
└── server/ # 服务端源码
  └── serverManager/     # 服务器管理模块
    ├── databaseManager/ # 数据库模块
    └── tcpServer/       # 网络通信模块

```