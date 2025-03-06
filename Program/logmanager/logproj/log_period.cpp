#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>
#include <atomic>
#include <csignal>
#include "log_client.h"

std::atomic<bool> running(true);  // 控制程序运行状态

int main() {
    LogClient client;
    int counter = 0;
    
    while (running) {
        // 生成不同类型的日志消息
        std::string msg;
        switch (counter % 4) {
            case 0: msg = "系统心跳检测正常"; break;
            case 1: msg = "CPU使用率: " + std::to_string(rand()%30 + 20) + "%"; break;
            case 2: msg = "内存剩余: " + std::to_string(rand()%2048 + 1024) + "MB"; break;
            case 3: msg = "网络延迟: " + std::to_string(rand()%50 + 10) + "ms"; break;
        }

        client.send_log("MonitorClient", msg);
        // std::cout << "已发送日志: " << msg << std::endl;
        
        ++counter;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    std::cout << "客户端已安全退出" << std::endl;
    return 0;
}