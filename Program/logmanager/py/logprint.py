import os
import socket
from datetime import datetime
import time

# 配置日志路径
log_dir = os.path.expanduser("/home/log")  # 自动处理路径格式
log_file = os.path.join(log_dir, "log.txt")
# print(f"[DEBUG] 日志目录绝对路径：{os.path.abspath(log_dir)}")  # 新增调试语句
# print(f"[DEBUG] 日志文件绝对路径：{os.path.abspath(log_file)}")  # 新增调试语句
hostname = socket.gethostname()       # 获取主机名
print(hostname)

# 创建日志目录（如果不存在）
os.makedirs(log_dir, exist_ok=True)

try:
    while True:
        # 生成精确到毫秒的时间戳
        now = datetime.now()
        timestamp = f"{now.strftime('%Y-%m-%d %H:%M:%S')}.{now.microsecond // 1000:03d}"
        
        # 构造日志内容
        log_entry = f"[{hostname}][{timestamp}]\n"
        
        # 写入文件（追加模式）
        with open(log_file, "a") as f:
            f.write(log_entry)
            f.flush()  # 强制立即写入
        
        # 等待1秒（实际间隔=1秒+代码执行时间）
        time.sleep(1)

except KeyboardInterrupt:
    print("\n日志记录已停止。")