#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void recv_handler(int sockfd) {
    char buffer[BUFFER_SIZE];
    while(true) {
        int valread = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if(valread <= 0) break;
        buffer[valread] = '\0';
        std::cout << "\nReceived: " << buffer << std::endl;
    }
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    // 转换IP地址
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }
    
    std::cout << "Connecting to server..." << std::endl;
    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }
    
    std::cout << "Connected! Start chatting (type 'exit' to quit)" << std::endl;
    
    // 启动接收线程
    std::thread recv_thread(recv_handler, sock);
    
    // 发送消息循环
    std::string message;
    while(true) {
        std::cout << "Enter message: ";
        std::getline(std::cin, message);
        if(message == "exit") break;
        send(sock, message.c_str(), message.size(), 0);
    }
    
    close(sock);
    recv_thread.detach();
    return 0;
}