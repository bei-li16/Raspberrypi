#include <libserial/SerialPort.h>
#include <iostream>
#include <string>
#include <csignal>
#include <cstdlib>

using namespace LibSerial;

// 全局串口对象
SerialPort serial_port;

// 信号处理函数（Ctrl+C 退出）
void signalHandler(int signum) 
{
    std::cout << "\n终止程序..." << std::endl;
    serial_port.Close();
    exit(signum);
}

int main() 
{
    // 注册信号处理
    signal(SIGINT, signalHandler);

    try {
        // 配置串口参数
        const std::string port_name = "/dev/ttyAMA0"; // 或 "/dev/serial0"
        const BaudRate baud_rate = BaudRate::BAUD_115200;

        // 打开串口
        serial_port.Open(port_name);
        if (!serial_port.IsOpen()) 
        {
            std::cerr << "无法打开串口: " << port_name << std::endl;
            return EXIT_FAILURE;
        }

        // 设置串口参数
        serial_port.SetBaudRate(baud_rate);
        serial_port.SetCharacterSize(CharacterSize::CHAR_SIZE_8);
        serial_port.SetFlowControl(FlowControl::FLOW_CONTROL_NONE);
        serial_port.SetParity(Parity::PARITY_NONE);
        serial_port.SetStopBits(StopBits::STOP_BITS_1);

        std::cout << "监听串口: " << port_name << " (波特率 " << " 115200 " << ")..." << std::endl;

        // 持续接收数据
        while (true) 
        {
            std::string data;
            try 
            {
                // 读取一行数据（直到遇到换行符或超时）
                serial_port.ReadLine(data, '\n', 1000); // 超时 1000ms
                if (!data.empty()) 
                {
                    std::cout << "接收数据: " << data << std::endl;
                }
            } catch (const ReadTimeout&) 
            {
                // 超时忽略，继续循环
            }
        }

    } catch (const OpenFailed&) 
    {
        std::cerr << "串口打开失败！检查设备权限或配置。" << std::endl;
        return EXIT_FAILURE;
    } catch (const std::exception& e) 
    {
        std::cerr << "错误: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}