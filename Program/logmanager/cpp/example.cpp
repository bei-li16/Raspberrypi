#include <iostream>
#include <unistd.h>
#include "log_client.h"

// 自动获取主机名函数
std::string get_hostname() 
{
    char hostname[256]{0};
    if (gethostname(hostname, sizeof(hostname)-1) != 0) 
    {
        throw std::runtime_error("Cannot get hostname");
    }
    return hostname;
}

int main() 
{
    std::cout << "start example" << std::endl;
    try 
    {
        const std::string host = get_hostname(); // 获取本机主机名
        
        // 双参数调用格式：log_print(主机名, 日志内容)
        log_print(host, "Application started");
        log_print(host, "User login: john");
        
        // 也可以直接指定主机名
        log_print("backup-server", "Database backup completed");
    } 
    catch(const std::exception& e) 
    {
        std::cerr << "Log Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}