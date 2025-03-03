# 编译服务端
g++ server.cpp -o server -pthread
# 编译客户端 
g++ client.cpp -o client -pthread

# 终端1：先运行服务端
./server

# 终端2：运行客户端
# ./client