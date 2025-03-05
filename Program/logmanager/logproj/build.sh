# g++ -o file_ops direxist.cpp
# ./file_ops

g++ -std=c++11 -o log_server log_server.cpp 
g++ -std=c++11 -o log_client log_client.cpp 
g++ log_uartprint.cpp  -lserial -o log_uartprint 
sudo ./log_server

# ./log_client