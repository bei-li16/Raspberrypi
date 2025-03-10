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
# 文件夹Raspberrypi及以下拷贝到Documents/下——>Documents/Raspberrypi/*
rsync -avz --progress /mnt/k/Data/GitFiles/Raspberrypi bei-li16@192.168.1.7:/home/bei-li16/Documents
rsync -avz --progress /mnt/k/Data/GitFiles/Raspberrypi bei-li16@192.168.1.7:/home/bei-li16/raspberrypiprogram
ssh-keygen -f ~/.ssh/known_hosts -R "192.168.1.7"   # 在本地计算机（WSL2）执行以下命令，清除旧密钥

参数说明：
-a：归档模式，保留权限、时间戳等。

-v：显示详细输出。

-z：压缩传输数据。

--progress：显示传输进度。
```

## git仓库同步

- 仓库地址https://github.com/bei-li16/Raspberrypi.git

## 树莓派指令

1. 基本指令
```bash
1. sudo raspi-config                # 修改树莓派配置
2. passwd || sudo passwd usr        # 修改用户登录密码
3. ip addr                          # 显示ip addr
4. gcc/g++ -v                       # 查看编译器版本和配置
5. sudo apt install libserial-dev -y    # 安装C++串口通信库
6. g++ uartprintcpp.cpp -lserial -o uartprintcpp    # 树莓派上进行C++编译 -lserial为链接库

7. g++ -std=c++17 -pthread log_service.cpp -o log_service
8. g++ -std=c++17 -fPIC -shared log_client.cpp -o liblogclient.so
9. g++ -Wl,-rpath='$ORIGIN' example.cpp -L. -llogclient -o example
scp bei-li16@192.168.1.7:/home/log/log.txt ./log.txt
cd Documents/Raspberrypi/Program/logmanager/logproj/
sudo chmod 777 /home/bei-li16/log/log_server.sock
tail -f /home/bei-li16/log/msg.log
sudo ./log_log_period
scp bei-li16@192.168.1.7:/home/bei-li16/log/*.log ./Program/logmanager/logproj/rasplog/
# 同步日志目录到本地的~/raspberry_logs目录
rsync -avz --progress bei-li16@192.168.1.7:/home/bei-li16/log/*.log ./Program/logmanager/logproj/rasplog/

```

3. 开启串口

```bash
1. sudo raspi-config                # 关闭串口到shell定向，开启硬件串口
2. sudo nano /boot/fireware/config.txt  # 添加下面两条指令
3. enable_uart=1                    # 开启串口
4. dtoverlay=disable-boot           # 禁用蓝牙
5. 
```