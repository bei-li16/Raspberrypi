import socket

SOCKET_PATH = "/tmp/log_service.sock"

def send_log(message):
    """发送日志到服务"""
    try:
        with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as s:
            s.connect(SOCKET_PATH)
            s.sendall(message.encode())
            response = s.recv(1024)
            return response == b"ACK"
    except Exception as e:
        print(f"发送失败: {str(e)}")
        return False

# 示例使用
if __name__ == "__main__":
    import sys
    if len(sys.argv) > 1:
        msg = " ".join(sys.argv[1:])
        if send_log(msg):
            print("日志发送成功")
        else:
            print("日志发送失败")