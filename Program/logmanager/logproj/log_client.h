#pragma once
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include <iomanip>
#include <sstream>
// 服务端代码示例（绑定套接字后）
#include <sys/stat.h>  // 需包含此头文件

#define SOCKET_PATH "/home/bei-li16/log/log_server.sock"

// 获取当前时间戳（精确到毫秒）
std::string get_current_time() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto now_time_t = system_clock::to_time_t(now);
    auto now_ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    std::tm tm_info;
    localtime_r(&now_time_t, &tm_info);  // 线程安全的时间转换

    std::stringstream ss;
    ss << std::put_time(&tm_info, "%Y-%m-%d %H:%M:%S") 
       << "." << std::setfill('0') << std::setw(3) << now_ms.count();
    return ss.str();
}

class LogClient {
private:
    int sock_fd = -1;  // 持久化连接
    struct sockaddr_un addr;

public:
    LogClient() {
        // 初始化地址
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    }

    bool connect() 
    {
        if (sock_fd != -1) return true;  // 已连接

        sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        // 创建并绑定套接字后，设置文件权限为 660（rw-rw----）
        if (chmod(SOCKET_PATH, 0660) == -1) {
            perror("chmod failed");
            exit(EXIT_FAILURE);
        }
        if (sock_fd == -1) 
        {
            perror("客户端 socket 创建失败");
            return false;
        }

        if (::connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) 
        {
            perror("连接服务端失败");
            close(sock_fd);
            sock_fd = -1;
            return false;
        }
        return true;
    }

    void send_log(const std::string &usrname, const std::string &msg)  
    {
        try 
        {
            // 发送日志...
            if (!connect()) return;

            // 格式化日志：[用户名][时间][消息]
            std::string log_entry = 
            "[" + usrname + "][" + get_current_time() + "]" + msg;
            if (write(sock_fd, log_entry.c_str(), log_entry.size()) == -1) 
            {
                perror("发送日志失败");
                close(sock_fd);
                sock_fd = -1;  // 标记需要重连
            }
            close(sock_fd);
            sock_fd = -1;  // 标记需要重连
        } 
        catch (const std::exception& e) 
        {
            std::cerr << "日志发送失败: " << e.what() << std::endl;
        }
    }

    ~LogClient() 
    {
        if (sock_fd != -1) close(sock_fd);
    }
};
