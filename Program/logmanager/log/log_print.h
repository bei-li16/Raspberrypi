#include <fstream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <sys/stat.h> // mkdir
#include <unistd.h>   // gethostname

class Logger {
public:
    Logger() {
        // 创建日志目录
        mkdir("/home/log", 0755); // 权限755
        
        // 打开日志文件（追加模式）
        logfile.open("/home/log/log.txt", std::ios::app);
        if (!logfile.is_open()) {
            throw std::runtime_error("无法打开日志文件");
        }
    }

    void log(const std::string& message) {
        // 获取主机名
        char hostname[256]{0};
        gethostname(hostname, sizeof(hostname)-1);

        // 获取当前时间（精确到毫秒）
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()
        ) % 1000;

        // 格式化输出
        logfile << "[" << hostname << "]["
                << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S")
                << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
                << message << std::endl;
    }

private:
    std::ofstream logfile;
};

// 全局单例
Logger& get_logger() {
    static Logger instance;
    return instance;
}

// 使用宏简化调用
#define LOG(msg) get_logger().log(msg)