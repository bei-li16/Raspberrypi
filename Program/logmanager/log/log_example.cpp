#include <thread>
#include "log_print.h"

int main() {
    // 示例日志
    LOG("程序启动");
    LOG("用户登录: john");
    
    // 多线程测试
    std::thread t1([]{
        LOG("线程1: 任务开始");
    });
    
    std::thread t2([]{
        LOG("线程2: 数据处理");
    });
    
    t1.join();
    t2.join();
    return 0;
}