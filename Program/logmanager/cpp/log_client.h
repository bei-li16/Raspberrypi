// log_client.h
#pragma once
#include <string>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <arpa/inet.h>
#include <iostream>
#include "log_config.h"

class Logger {
public:
    Logger() 
    {
        std::cout << "Logger init ... " << std::endl;
        connect();
    }

    // 修改为接收hostname和message双参数
    void log_print(const std::string& hostname, const std::string& message)
    {
        std::cout << "log_print" << hostname << message << std::endl;
        // 组合协议格式：主机名长度(4字节) + 主机名 + 消息
        uint32_t host_len = htonl(hostname.size());
        std::string payload;
        payload.append(reinterpret_cast<char*>(&host_len), 4);
        payload.append(hostname);
        payload.append(message);
        for(int i=0; i<3; ++i) 
        {
            try 
            {
                send_message(payload);
                return;
            }
            catch(const std::exception&) 
            {
                if(i == 2) throw;
                reconnect();
            }
        }
    }

private:
    int sock_fd = -1;
    sockaddr_un addr{};

    void connect() 
    {
        sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path)-1);

        if(::connect(sock_fd, (sockaddr*)&addr, sizeof(addr)) < 0) 
        {
            throw std::runtime_error("Connection failed");
        }
    }

    void send_message(const std::string& msg) 
    {
        if(write(sock_fd, msg.c_str(), msg.size()) < 0) 
        {
            throw std::runtime_error("Send failed");
        }
        else
        {
            std::cout << "send_message success. " << std::endl;
        }

        char ack[3];
        if(read(sock_fd, ack, 3) != 3 || memcmp(ack, "ACK", 3) != 0) 
        {
            throw std::runtime_error("ACK mismatch");
        }
        else
        {
            std::cout << "send_message ACK match. " << std::endl;
        }
    }

    void reconnect() 
    {
        std::cout << "reconnect ... " << std::endl;
        close(sock_fd);
        connect();
    }
};

// 全局单例
extern Logger& get_logger();

// 宏定义简化调用
#define log_print(host, msg) get_logger().log_print(host, msg)
