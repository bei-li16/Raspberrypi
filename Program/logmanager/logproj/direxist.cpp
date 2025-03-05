#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <fstream>
#include <string>

std::string path = "./log/msg.log";

// 检查路径是否存在（文件或目录）
bool pathExists(const std::string& path) 
{
    struct stat info;
    return (stat(path.c_str(), &info) == 0);
}

// 判断是否为目录
bool isDirectory(const std::string& path) 
{
    struct stat info;
    if (stat(path.c_str(), &info)) return false;
    return S_ISDIR(info.st_mode);
}

// 判断是否为文件
bool isFile(const std::string& path) 
{
    struct stat info;
    if (stat(path.c_str(), &info)) return false;
    return S_ISREG(info.st_mode);
}

// 创建目录（仅单级目录）
bool createDirectory(const std::string& path) 
{
    if (mkdir(path.c_str(), 0755) == 0) return true; // 权限 0755 = rwxr-xr-x
    if (errno == EEXIST) return true;                // 目录已存在
    return false;
}

// 递归创建目录（支持多级目录）
bool createDirectories(const std::string& path) 
{
    size_t pos = 0;
    while ((pos = path.find_first_of('/', pos + 1)) != std::string::npos) 
    {
        std::string subdir = path.substr(0, pos);
        if (subdir.empty()) continue; // 跳过根目录
        if (!createDirectory(subdir) && errno != EEXIST) return false;
    }
    return createDirectory(path);
}

// 创建文件并打开（若文件已存在则覆盖）
bool createFile(const std::string& filepath) 
{
    std::ofstream file(filepath);
    return file.is_open();
}

// 安全创建文件（若不存在则创建，存在则追加）
bool openOrCreateFile(const std::string& filepath) 
{
    std::ofstream file;
    if (pathExists(filepath)) 
    {
        file.open(filepath, std::ios::app); // 追加模式打开
    } 
    else 
    {
        file.open(filepath);                // 创建并打开
    }
    return file.is_open();
}

int EnsureWritefile(std::string path)
{
    // 检查路径是否存在
    if (pathExists(path)) 
    {
        if (isDirectory(path)) 
        {
            std::cout << "目录已存在: " << path << std::endl;
        } 
        else if (isFile(path)) 
        {
            std::cout << "文件已存在: " << path << std::endl;
            // std::ifstream file(path);
            // 读取或处理文件...
        }
    } 
    else 
    {
        // 递归创建父目录
        std::string parent_dir = path.substr(0, path.find_last_of('/'));
        if (!parent_dir.empty() && !createDirectories(parent_dir)) 
        {
            std::cerr << "创建目录失败: " << parent_dir << std::endl;
            return 1;
        }

        // 创建文件
        if (openOrCreateFile(path)) {
            std::cout << "文件创建成功: " << path << std::endl;
        } else {
            std::cerr << "文件创建失败: " << path << std::endl;
            return 1;
        }
    }

    return 0;
}

int main()
{
    EnsureWritefile(path);
    return 0;
}
