#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include <iomanip>
#include <sstream>

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

    bool connect() {
        if (sock_fd != -1) return true;  // 已连接

        sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock_fd == -1) {
            perror("客户端 socket 创建失败");
            return false;
        }

        if (::connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
            perror("连接服务端失败");
            close(sock_fd);
            sock_fd = -1;
            return false;
        }
        return true;
    }

    void send_log(const std::string &usrname, const std::string &msg)  {
        if (!connect()) return;

        // // 格式化日志：[用户名][时间][消息]
        std::string log_entry = 
        "[" + usrname + "][" + get_current_time() + "]" + msg;
        if (write(sock_fd, log_entry.c_str(), log_entry.size()) == -1) {
            perror("发送日志失败");
            close(sock_fd);
            sock_fd = -1;  // 标记需要重连
        }
        close(sock_fd);
        sock_fd = -1;  // 标记需要重连
    }

    ~LogClient() {
        if (sock_fd != -1) close(sock_fd);
    }
};

int main() 
{
    // 使用方式
    LogClient client;
    int i = 0;
    // while (1)
    // {
    //     while (i%10000000 == 0)
    //     {
            client.send_log("AA", "日志1\n");
            client.send_log("AA", "日志2\n");  // 复用同一连接
            client.send_log("AA", "这是一条测试日志\n");
            client.send_log("raspberrypi4b", "系统启动完成\n");
            client.send_log("raspberrypi4b", "检测到温度过高: 75℃\n");
    //     }
    //     i++;
    // }
    return 0;
}