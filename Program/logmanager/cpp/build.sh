g++ -std=c++17 -pthread log_service.cpp -o log_service
g++ -std=c++17 -fPIC -shared log_client.cpp -o liblogclient.so
g++ -Wl,-rpath='$ORIGIN' example.cpp -L. -llogclient -o example
sudo pkill -9 log_service
ps aux | grep log_service
sudo rm -f /tmp/log_service.sock
sudo ./log_service
sudo ./example