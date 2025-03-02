# Raspberrypi

## WSL2中打开工程文件夹
- cd /mnt/k/Data/GitFiles/Raspberrypi   #盘符小写，没有冒号，使用反斜杠

## WSL2文件同步至树莓派
- 使用 rsync 命令 增量同步

```bash
# WSL 中安装
sudo apt update && sudo apt install rsync

# 树莓派中安装
sudo apt update && sudo apt install rsync
```
- 命令同步

```bash
rsync -avz --progress /home/bei-li16/program/ bei-li16@192.168.1.2:/home/bei-li16/program/

参数说明：
-a：归档模式，保留权限、时间戳等。

-v：显示详细输出。

-z：压缩传输数据。

--progress：显示传输进度。
```

## git仓库同步

- 仓库地址https://github.com/bei-li16/Raspberrypi.git

## 树莓派指令

```bash
1. sudo raspi-config                # 修改树莓派配置
2. passwd || sudo passwd usr        # 修改用户登录密码
3. ip addr                          # 显示ip addr
```