import serial
import sys
import os
import socket
from datetime import datetime
import time

# 配置串口参数
SERIAL_PORT = "/dev/ttyAMA0"  # 树莓派3/4默认硬件串口
BAUD_RATE = 115200              # 波特率需与发送端一致

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
    # 初始化串口
    ser = serial.Serial(
        port=SERIAL_PORT,
        baudrate=BAUD_RATE,
        bytesize=serial.EIGHTBITS,    # 数据位
        parity=serial.PARITY_NONE,    # 校验位
        stopbits=serial.STOPBITS_ONE, # 停止位
        timeout=1                     # 读取超时时间（秒）
    )

    print(f"正在监听串口 {SERIAL_PORT}，波特率 {BAUD_RATE}...")
    
    while True:
        # 生成精确到毫秒的时间戳
        now = datetime.now()
        timestamp = f"{now.strftime('%Y-%m-%d %H:%M:%S')}.{now.microsecond // 1000:03d}"
        # 构造日志内容
        log_entry = f"[{hostname}][{timestamp}]"
        
        try:
            # 读取一行数据（直到遇到换行符）
            data = ser.readline().decode('utf-8').strip()
            if data:
                log_entry = log_entry + "[MCU]" + data + "\n"
                # print(f"接收数据: {data}")
                # print(f"log_entry: {log_entry}")
                # 写入文件（追加模式）
                with open(log_file, "a") as f:
                    f.write(log_entry)
                    f.flush()  # 强制立即写入
        except UnicodeDecodeError:
            print("接收非UTF-8数据（原始字节）:", ser.readline().strip())
        
        # 写入文件（追加模式）
        # with open(log_file, "a") as f:
        #     f.write(log_entry)
        #     f.flush()  # 强制立即写入
        
        # 等待1秒（实际间隔=1秒+代码执行时间）
        # time.sleep(1)
    
    # 关闭串口（此处不会执行，需手动终止脚本）
    ser.close()

except serial.SerialException as e:
    print(f"无法打开串口: {e}")
    sys.exit(1)

except KeyboardInterrupt:
    print("\n日志记录已停止。")