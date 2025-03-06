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
#include <sys/stat.h>
#include <cstdio>

#define SOCKET_PATH         "/home/bei-li16/log/log_server.sock"
#define LOG_FILE            "/home/bei-li16/log/msg.log"
#define THREAD_POOL_SIZE    4
#define MAX_LOG_SIZE        (1 * 1024 * 1024)  // 1MB
#define MAX_LOG_FILES       10

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

<<<<<<< HEAD
// 日志写入线程
void log_writer() 
{
    std::ofstream log_file(LOG_FILE, std::ios::app);
=======
// 获取文件大小
off_t get_file_size(const std::string& path) 
{
    struct stat st;
    return (stat(path.c_str(), &st) == 0) ? st.st_size : -1;
}

// 日志轮换逻辑
void rotate_logs() 
{
    const std::string base = std::string(LOG_FILE).substr(0, std::string(LOG_FILE).rfind(".log"));
    const std::string ext = ".log";

    // 删除最旧日志
    std::string oldest = base + std::to_string(MAX_LOG_FILES) + ext;
    if (remove(oldest.c_str()) != 0 && errno != ENOENT) 
    {
        perror(("删除失败: " + oldest).c_str());
    }

    // 逆向轮换文件
    for (int i = MAX_LOG_FILES-1; i >= 1; --i) 
    {
        std::string old_name = base + std::to_string(i) + ext;
        std::string new_name = base + std::to_string(i+1) + ext;
        if (rename(old_name.c_str(), new_name.c_str()) != 0 && errno != ENOENT) 
        {
            perror(("重命名失败: " + old_name + " -> " + new_name).c_str());
        }
    }

    // 轮换当前日志
    std::string first_rotated = base + "1" + ext;
    if (rename(LOG_FILE, first_rotated.c_str()) != 0 && errno != ENOENT) 
    {
        perror("当前日志轮换失败");
    }
}

// 日志写入线程
void log_writer() {
    std::ofstream log_file;
    log_file.open(LOG_FILE, std::ios::app);

>>>>>>> d32864e ([modify] uart 日志正常)
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
<<<<<<< HEAD
            std::time_t now = std::time(nullptr);
            log_file << log_entry << std::endl;
=======
            // 写入日志
            log_file << log_entry;
            log_file.flush();

            // 检查文件大小
            if (get_file_size(LOG_FILE) >= MAX_LOG_SIZE) 
            {
                log_file.close();
                rotate_logs();
                log_file.open(LOG_FILE, std::ios::app);
                if (!log_file.is_open()) 
                {
                    std::cerr << "重新打开日志文件失败！" << std::endl;
                    return;
                }
            }
>>>>>>> d32864e ([modify] uart 日志正常)
        }
    }
    log_file.close();
}

<<<<<<< HEAD
=======
// // 日志写入线程
// void log_writer() 
// {
//     std::ofstream log_file(LOG_FILE, std::ios::app);
//     if (!log_file.is_open()) 
//     {
//         std::cerr << "无法打开日志文件！" << std::endl;
//         return;
//     }

//     while (running || !log_queue.empty()) 
//     {
//         std::string log_entry;
//         if (log_queue.pop(log_entry)) 
//         {
//             std::time_t now = std::time(nullptr);
//             // 服务端写入逻辑修改
//             log_file << log_entry;       // 直接写入客户端原始内容
//             log_file.flush();            // 手动刷新缓冲区
//         }
//     }
//     log_file.close();
// }

>>>>>>> d32864e ([modify] uart 日志正常)
// 在服务端的 handle_client 函数中添加日志
void handle_client(int client_fd) 
{
    char buffer[1024];
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer));
    if (bytes_read > 0) 
    {
        std::string message(buffer, bytes_read);
<<<<<<< HEAD
        std::cout << "接收到日志: " << message << std::endl;  // 调试输出
=======
        // std::cout << "接收到日志: " << message << std::endl;  // 调试输出
>>>>>>> d32864e ([modify] uart 日志正常)
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