# OpenClaw 部署指南

> 在树莓派上部署 OpenClaw AI Bot 的完整指南
> 支持单渠道（飞书/企微）和多渠道混合部署

---

## 目录

1. [环境准备](#1-环境准备)
2. [基础部署](#2-基础部署)
3. [方案一：双实例隔离部署](#3-方案一双实例隔离部署)
4. [方案二：单实例多渠道部署](#4-方案二单实例多渠道部署)
5. [企业微信配置](#5-企业微信配置)
6. [故障排查](#6-故障排查)

---

## 1. 环境准备

### 1.1 硬件要求

- 树莓派 3B+/4B（推荐 4B 2GB+）
- SD 卡 16GB+
- 稳定的网络连接

### 1.2 系统要求

- Raspberry Pi OS (64-bit) 或 Ubuntu 22.04 LTS
- Node.js 18+ 和 npm

### 1.3 安装依赖

```bash
# 更新系统
sudo apt update && sudo apt upgrade -y

# 安装 Node.js 18
curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -
sudo apt install -y nodejs

# 验证安装
node --version  # v18.x.x
npm --version   # 9.x.x

# 安装 OpenClaw
npm install -g openclaw

# 验证安装
openclaw --version
```

### 1.4 准备材料

部署前请准备好以下信息：

| 渠道 | 所需材料 |
|------|---------|
| **飞书** | App ID, App Secret, Encrypt Key, Verification Token |
| **企业微信** | CorpID, AgentID, Secret |
| **DeepSeek** | API Key (从 [platform.deepseek.com](https://platform.deepseek.com) 获取) |
| **Moonshot** | API Key (从 [platform.moonshot.cn](https://platform.moonshot.cn) 获取) |

---

## 2. 基础部署

### 2.1 初始化 OpenClaw

```bash
# 创建配置目录
mkdir -p ~/.openclaw

# 初始化配置
openclaw init
```

这会创建以下目录结构：

```
~/.openclaw/
├── config.yaml      # 主配置文件
├── workspace/       # 工作目录
├── logs/           # 日志文件
└── plugins/        # 插件目录
```

### 2.2 基础配置文件

编辑 `~/.openclaw/config.yaml`：

```yaml
# 网关配置
gateway:
  port: 8080
  host: 0.0.0.0

# 渠道配置
channels:
  feishu:
    type: feishu
    app-id: "你的飞书AppID"
    app-secret: "你的飞书AppSecret"
    encrypt-key: "你的EncryptKey"
    verification-token: "你的VerificationToken"

# 模型配置
models:
  kimi:
    provider: moonshot
    api-key: "你的Moonshot API Key"
    model: kimi-k2.5

# 路由配置
routing:
  default:
    channel: feishu
    model: kimi
```

### 2.3 启动服务

**方式一：直接启动（测试用）**

```bash
openclaw gateway start
```

**方式二：Systemd 服务（推荐）**

```bash
# 创建服务文件
sudo nano /etc/systemd/system/openclaw.service
```

内容：

```ini
[Unit]
Description=OpenClaw Gateway
After=network.target

[Service]
Type=simple
User=pi
ExecStart=/usr/bin/openclaw gateway start
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```

启用并启动：

```bash
sudo systemctl daemon-reload
sudo systemctl enable openclaw
sudo systemctl start openclaw

# 查看状态
sudo systemctl status openclaw

# 查看日志
sudo journalctl -u openclaw -f
```

---

## 3. 方案一：双实例隔离部署

> 两个独立的 OpenClaw 进程，完全隔离，互不影响
> - 实例1：飞书 + Moonshot/Kimi
> - 实例2：企业微信 + DeepSeek

### 3.1 方案特点

| 优点 | 缺点 |
|------|------|
| 故障隔离，一个崩了不影响另一个 | 资源占用稍高（多一个进程） |
| 配置简单，各自独立 | 需要管理两个服务 |
| 升级维护可独立进行 | 端口需要错开 |
| 日志分离，排查方便 | - |

### 3.2 部署步骤

#### 步骤1：确保飞书实例正常运行

飞书实例使用默认配置，端口 8080。

#### 步骤2：创建企微实例目录

```bash
# 创建独立工作目录
mkdir -p ~/.openclaw-wechat
export OPENCLAW_HOME=~/.openclaw-wechat

# 初始化配置
openclaw init
```

#### 步骤3：配置企微实例

编辑 `~/.openclaw-wechat/config.yaml`：

```yaml
# 网关配置 - 端口与飞书错开
gateway:
  port: 8081
  host: 0.0.0.0

# 企业微信渠道配置
channels:
  wechat-work:
    type: wechat-work
    corp-id: "你的CorpID"
    agent-id: "你的AgentID"
    secret: "你的Secret"
    token: "随机16位字符串"
    encoding-aes-key: "43位随机字符串"

# DeepSeek 模型配置
models:
  deepseek:
    provider: deepseek
    api-key: "你的DeepSeek API Key"
    model: deepseek-chat  # 或 deepseek-reasoner

# 路由配置
routing:
  default:
    channel: wechat-work
    model: deepseek
```

#### 步骤4：生成随机字符串

```bash
# 生成16位 Token
openssl rand -base64 12 | cut -c1-16

# 生成43位 EncodingAESKey
openssl rand -base64 32 | cut -c1-43
```

#### 步骤5：创建 Systemd 服务

```bash
sudo nano /etc/systemd/system/openclaw-wechat.service
```

内容：

```ini
[Unit]
Description=OpenClaw WeChat Work Bot
After=network.target

[Service]
Type=simple
User=pi
Environment="OPENCLAW_HOME=/home/pi/.openclaw-wechat"
ExecStart=/usr/bin/openclaw gateway start --home /home/pi/.openclaw-wechat
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```

#### 步骤6：启动服务

```bash
sudo systemctl daemon-reload
sudo systemctl enable openclaw-wechat
sudo systemctl start openclaw-wechat

# 查看状态
sudo systemctl status openclaw-wechat
```

### 3.3 端口规划

| 实例 | 端口 | 用途 |
|------|------|------|
| openclaw | 8080 | 飞书 + Kimi |
| openclaw-wechat | 8081 | 企业微信 + DeepSeek |

### 3.4 管理命令

```bash
# 查看飞书实例状态
sudo systemctl status openclaw

# 查看企微实例状态
sudo systemctl status openclaw-wechat

# 重启飞书实例
sudo systemctl restart openclaw

# 重启企微实例
sudo systemctl restart openclaw-wechat

# 查看飞书日志
sudo journalctl -u openclaw -f

# 查看企微日志
sudo journalctl -u openclaw-wechat -f
```

---

## 4. 方案二：单实例多渠道部署

> 一个 OpenClaw 进程同时对接飞书和企业微信
> 通过路由规则将不同渠道映射到不同模型

### 4.1 方案特点

| 优点 | 缺点 |
|------|------|
| 资源占用低，单进程 | 配置相对复杂 |
| 统一管理，一个服务 | 故障不隔离，崩了都崩 |
| 端口只需一个 | 日志混在一起 |
| - | 升级时两个渠道都中断 |

### 4.2 配置文件

编辑 `~/.openclaw/config.yaml`：

```yaml
# 网关配置
gateway:
  port: 8080
  host: 0.0.0.0

# 多渠道配置
channels:
  # 飞书渠道
  feishu:
    type: feishu
    app-id: "你的飞书AppID"
    app-secret: "你的飞书AppSecret"
    encrypt-key: "你的EncryptKey"
    verification-token: "你的VerificationToken"
  
  # 企业微信渠道
  wechat-work:
    type: wechat-work
    corp-id: "你的CorpID"
    agent-id: "你的AgentID"
    secret: "你的Secret"
    token: "随机16位字符串"
    encoding-aes-key: "43位随机字符串"

# 多模型配置
models:
  # Kimi 模型
  kimi:
    provider: moonshot
    api-key: "你的Moonshot API Key"
    model: kimi-k2.5
  
  # DeepSeek 模型
  deepseek:
    provider: deepseek
    api-key: "你的DeepSeek API Key"
    model: deepseek-chat

# 路由配置 - 按渠道路由到不同模型
routing:
  # 飞书 → Kimi
  feishu:
    channel: feishu
    model: kimi
  
  # 企业微信 → DeepSeek
  wechat-work:
    channel: wechat-work
    model: deepseek
```

### 4.3 启动服务

```bash
# 重启服务使配置生效
sudo systemctl restart openclaw

# 查看状态
sudo systemctl status openclaw

# 查看日志
sudo journalctl -u openclaw -f
```

### 4.4 路由规则说明

路由配置支持多种匹配方式：

```yaml
routing:
  # 按渠道匹配
  feishu:
    channel: feishu
    model: kimi
  
  # 按用户ID匹配（特定用户使用特定模型）
  "user:ou_xxx":
    channel: feishu
    model: deepseek
  
  # 默认路由
  default:
    channel: feishu
    model: kimi
```

---

## 5. 企业微信配置

### 5.1 创建自建应用

1. 登录 [企业微信管理后台](https://work.weixin.qq.com/wework_admin)
2. 进入「应用管理」
3. 点击「创建应用」
4. 填写应用信息：
   - 应用名称：AI 助手（或自定义）
   - 应用图标：上传自定义图标
   - 可见成员：选择可见范围

### 5.2 获取关键参数

创建完成后，在应用详情页获取：

| 参数 | 位置 |
|------|------|
| AgentID | 应用详情页顶部 |
| Secret | 点击「查看」获取 |

在「我的企业」页面获取：

| 参数 | 位置 |
|------|------|
| CorpID | 我的企业 → 企业ID |

### 5.3 配置接收消息

在应用详情页，点击「接收消息」→「设置」：

| 参数 | 值 |
|------|-----|
| URL | `https://你的域名/wechat-work` |
| Token | 上面生成的16位随机字符串 |
| EncodingAESKey | 上面生成的43位随机字符串 |
| 消息加解密方式 | 推荐「安全模式」|

### 5.4 配置可信域名

在应用详情页，点击「网页授权及JS-SDK」：

1. 设置可信域名：你的域名
2. 下载验证文件并放到网站根目录
3. 点击「申请校验域名」

### 5.5 网络要求

企业微信要求回调 URL 必须：
- 使用 HTTPS
- 使用标准 443 端口（或配置反向代理）
- 域名已备案（国内服务器）

#### 网络方案选择

| 方案 | 适用场景 | 配置难度 |
|------|---------|---------|
| **Tailscale + HTTPS** | 已有 Tailnet | 低 |
| **Cloudflare Tunnel** | 无公网 IP | 低 |
| **frp/ngrok** | 临时测试 | 中 |
| **Nginx 反向代理** | 有公网服务器 | 中 |

#### Nginx 反向代理示例

```nginx
server {
    listen 443 ssl;
    server_name your-domain.com;

    ssl_certificate /path/to/cert.pem;
    ssl_certificate_key /path/to/key.pem;

    location /wechat-work {
        proxy_pass http://localhost:8081;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }
}
```

---

## 6. 故障排查

### 6.1 常见问题

#### 问题1：服务无法启动

```bash
# 检查配置文件语法
openclaw config validate

# 查看详细错误
openclaw gateway start --verbose
```

#### 问题2：企业微信回调验证失败

- 检查 URL 是否可公网访问
- 检查 Token 和 EncodingAESKey 是否匹配
- 检查 CorpID、AgentID、Secret 是否正确

#### 问题3：模型无响应

```bash
# 测试模型连接
openclaw model test --model deepseek

# 检查 API Key 是否有效
```

#### 问题4：端口冲突

```bash
# 查看端口占用
sudo lsof -i :8080
sudo lsof -i :8081

# 更换端口后重启服务
```

### 6.2 日志查看

```bash
# 查看 OpenClaw 日志
sudo journalctl -u openclaw -f

# 查看企微实例日志（方案一）
sudo journalctl -u openclaw-wechat -f

# 查看所有日志
sudo journalctl -u 'openclaw*' -f
```

### 6.3 调试模式

```bash
# 前台运行，查看实时日志
openclaw gateway start --verbose

# 指定配置文件运行
openclaw gateway start --config /path/to/config.yaml
```

---

## 附录

### A. 目录结构参考

```
~/
├── .openclaw/                    # 飞书实例（默认）
│   ├── config.yaml
│   ├── workspace/
│   └── logs/
│
├── .openclaw-wechat/             # 企微实例（方案一）
│   ├── config.yaml
│   ├── workspace/
│   └── logs/
│
└── Project/
    └── Raspberrypi/
        └── Program/
            └── OpenClaw/         # 本指南所在目录
                └── DEPLOY.md
```

### B. 快速命令参考

```bash
# 状态检查
openclaw status
openclaw status --home ~/.openclaw-wechat

# 配置验证
openclaw config validate
openclaw config validate --home ~/.openclaw-wechat

# 服务管理
sudo systemctl {start|stop|restart|status} openclaw
sudo systemctl {start|stop|restart|status} openclaw-wechat
```

### C. 相关链接

- [OpenClaw 文档](https://docs.openclaw.ai)
- [OpenClaw GitHub](https://github.com/openclaw/openclaw)
- [DeepSeek 平台](https://platform.deepseek.com)
- [Moonshot 平台](https://platform.moonshot.cn)
- [企业微信开发者中心](https://developer.work.weixin.qq.com)

---

> 最后更新：2026-03-16
> 适用版本：OpenClaw 最新版
