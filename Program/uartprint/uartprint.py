import serial
import sys

# 配置串口参数
SERIAL_PORT = "/dev/ttyAMA0"  # 树莓派3/4默认硬件串口
BAUD_RATE = 115200              # 波特率需与发送端一致

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

    # 持续接收数据
    while True:
        try:
            # 读取一行数据（直到遇到换行符）
            data = ser.readline().decode('utf-8').strip()
            if data:
                print(f"接收数据: {data}")
        except UnicodeDecodeError:
            print("接收非UTF-8数据（原始字节）:", ser.readline().strip())

    # 关闭串口（此处不会执行，需手动终止脚本）
    ser.close()

except serial.SerialException as e:
    print(f"无法打开串口: {e}")
    sys.exit(1)
except KeyboardInterrupt:
    print("\n用户终止脚本")
    ser.close()
    sys.exit(0)