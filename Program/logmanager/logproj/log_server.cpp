#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fstream>
#include <ctime>
#include <vector>


#define SOCKET_PATH "/home/bei-li16/log/log_server.sock"
#define LOG_FILE    "/home/bei-li16/log/msg.log"
#define THREAD_POOL_SIZE 4

// 线程安全队列
template<typename T>
class ThreadSafeQueue 
{
private:
    std::queue<T> queue;
    std::mutex mtx;
    std::condition_variable cv;

public:
    void push(const T& value) 
    {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(value);
        cv.notify_one();
    }

    bool pop(T& value) 
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !queue.empty(); });
        value = queue.front();
        queue.pop();
        return true;
    }

    bool empty() 
    {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.empty();
    }
};

// 全局变量
ThreadSafeQueue<std::string> log_queue;
std::atomic<bool> running{true};

// 日志写入线程
void log_writer() 
{
    std::ofstream log_file(LOG_FILE, std::ios::app);
    if (!log_file.is_open()) 
    {
        std::cerr << "无法打开日志文件！" << std::endl;
        return;
    }

    while (running || !log_queue.empty()) 
    {
        std::string log_entry;
        if (log_queue.pop(log_entry)) 
        {
            std::time_t now = std::time(nullptr);
            log_file << log_entry << std::endl;
        }
    }
    log_file.close();
}

// 在服务端的 handle_client 函数中添加日志
void handle_client(int client_fd) 
{
    char buffer[1024];
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer));
    if (bytes_read > 0) 
    {
        std::string message(buffer, bytes_read);
        std::cout << "接收到日志: " << message << std::endl;  // 调试输出
        log_queue.push(message);
    } 
    else 
    {
        std::cerr << "读取客户端数据失败或连接关闭" << std::endl;
    }
    close(client_fd);
}

int main() 
{
    unlink(SOCKET_PATH);
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) 
    {
        perror("socket 创建失败");
        return 1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) 
    {
        perror("绑定失败");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, SOMAXCONN) == -1) 
    {
        perror("监听失败");
        close(server_fd);
        return 1;
    }

    // 启动日志写入线程
    std::thread writer_thread(log_writer);

    // 创建工作线程池（修复部分）
    std::vector<std::thread> worker_pool;
    for (int i = 0; i < THREAD_POOL_SIZE; ++i) 
    {
        worker_pool.emplace_back([server_fd] {  // 捕获 server_fd
            while (running) 
            {
                int client_fd = accept(server_fd, nullptr, nullptr);
                if (client_fd == -1) continue;
                handle_client(client_fd);
            }
        });
    }

    std::cout << "日志服务运行中 (按 Enter 停止)...\n";
    std::cin.get();
    running = false;

    // 清理资源
    for (auto& t : worker_pool) t.join();
    writer_thread.join();
    close(server_fd);
    unlink(SOCKET_PATH);

    return 0;
}