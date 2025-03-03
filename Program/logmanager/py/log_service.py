import socket
import threading
import queue
from pathlib import Path

# 配置参数
LOG_FILE = Path("/home/log/log.txt").expanduser()
SOCKET_PATH = "/tmp/log_service.sock"
MAX_QUEUE_SIZE = 1000  # 最大队列容量

# 初始化队列和锁
log_queue = queue.Queue(maxsize=MAX_QUEUE_SIZE)
write_lock = threading.Lock()

def write_log_worker():
    # """日志写入工作线程"""
    while True:
        try:
            # 从队列获取日志（阻塞模式）
            message = log_queue.get()
            
            # 写入文件（原子操作）
            with write_lock, open(LOG_FILE, "a") as f:
                f.write(message + "\n")
                f.flush()
                
            log_queue.task_done()
        except Exception as e:
            print(f"[Error] 写入失败: {str(e)}")

def handle_client(conn):
    # """处理客户端连接"""
    try:
        while True:
            data = conn.recv(1024).decode()
            if not data:
                break
                
            # 添加日志到队列（非阻塞模式）
            try:
                log_queue.put_nowait(data)
            except queue.Full:
                conn.sendall(b"ERROR: Queue full")
                break
            else:
                conn.sendall(b"ACK")
    finally:
        conn.close()

def start_service():
    # """启动日志服务"""
    # 确保日志目录存在
    LOG_FILE.parent.mkdir(parents=True, exist_ok=True)
    
    # 启动写入线程
    threading.Thread(target=write_log_worker, daemon=True).start()
    
    # 创建Unix域套接字
    Path(SOCKET_PATH).unlink(missing_ok=True)
    
    with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as s:
        s.bind(SOCKET_PATH)
        s.listen()
        print(f"Log service started at {SOCKET_PATH}")
        
        while True:
            conn, _ = s.accept()
            threading.Thread(target=handle_client, args=(conn,)).start()

if __name__ == "__main__":
    start_service()