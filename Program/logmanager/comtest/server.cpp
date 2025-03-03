#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

std::mutex cout_mtx; // 保护标准输出

void client_handler(int sockfd, int client_id) {
    char buffer[BUFFER_SIZE];
    
    {
        std::lock_guard<std::mutex> lock(cout_mtx);
        std::cout << "Client " << client_id << " connected" << std::endl;
    }

    while(true) {
        int valread = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if(valread <= 0) break;
        buffer[valread] = '\0';
        
        {
            std::lock_guard<std::mutex> lock(cout_mtx);
            std::cout << "[Client " << client_id << "] " << buffer << std::endl;
        }

        // 广播消息给所有客户端
        std::string response = "Client " + std::to_string(client_id) + ": " + buffer;
        send(sockfd, response.c_str(), response.size(), 0); // 单播
    }

    {
        std::lock_guard<std::mutex> lock(cout_mtx);
        std::cout << "Client " << client_id << " disconnected" << std::endl;
    }
    close(sockfd);
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    // 创建套接字
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // 设置套接字选项
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // 绑定套接字
    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // 开始监听
    if(listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Multi-client server started on port " << PORT << std::endl;
    std::cout << "Waiting for connections..." << std::endl;

    std::vector<std::thread> clients;
    int client_counter = 0;

    while(true) {
        int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if(new_socket < 0) {
            perror("accept error");
            continue;
        }

        // 显示客户端IP
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &address.sin_addr, client_ip, INET_ADDRSTRLEN);
        std::cout << "New connection from: " << client_ip << std::endl;

        // 创建客户端线程
        clients.emplace_back(client_handler, new_socket, ++client_counter);
        clients.back().detach();
    }

    close(server_fd);
    return 0;
}