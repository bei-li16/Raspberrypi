// log_service.cpp
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <csignal>
#include <iostream>
#include "log_config.h"


std::queue<std::string> log_queue;
std::mutex queue_mutex;
std::condition_variable queue_cv;
bool running = true;

void signal_handler(int) {
    running = false;
    queue_cv.notify_all();
}

void write_worker() 
{
    std::ofstream logfile;
    logfile.open("/home/log/log.txt", std::ios::app);

    while(running || !log_queue.empty()) 
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        queue_cv.wait(lock, []{ 
            return !log_queue.empty() || !running;
        });

        if(!log_queue.empty()) 
        {
            auto msg = log_queue.front();
            log_queue.pop();
            lock.unlock();

            logfile << msg << std::endl;
            logfile.flush();
        }
    }
    std::cout << "write_worker out" << std::endl;
    logfile.close();
}

int main() {
    // 守护进程化
    //daemon(1, 0);
    
    std::cout << "start log_service" << std::endl;
    // 信号处理
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    // 启动写入线程
    std::thread worker(write_worker);

    // 创建UNIX域套接字
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddr_un addr{.sun_family=AF_UNIX};
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path)-1);

    unlink(SOCKET_PATH);
    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);

    // 主接受循环
    while(running) 
    {
        int client_fd = accept(server_fd, NULL, NULL);
        if(client_fd < 0) continue;

        char buffer[4096];
        ssize_t bytes = read(client_fd, buffer, sizeof(buffer)-1);

        std::cout << "Recieve msg in begin" << bytes << std::endl;
        if(bytes > 0) 
        {
            std::cout << "Recieve msg in" << bytes << std::endl;
            buffer[bytes] = 0;
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                if(log_queue.size() < MAX_QUEUE_SIZE) 
                {
                    log_queue.push(buffer);
                    queue_cv.notify_one();
                }
            }
            write(client_fd, "ACK", 3);
        }
        close(client_fd);
    }

    std::cout << "log_service end." << std::endl;
    // 清理
    close(server_fd);
    unlink(SOCKET_PATH);
    worker.join();
    return 0;
}