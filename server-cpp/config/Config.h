#ifndef CONFIG_H
#define CONFIG_H

#include "../server/Server.h"

using namespace std;

class Config
{
public:
    Config();
    ~Config(){};

    void parse_arg(int argc, char*argv[]);

    // 服务器 ip 地址
    string serverIpAddr;

    // 端口号
    int PORT;

    // 数据库 ip 地址
    string databaseIpAddr;

    // 数据库用户名
    string databaseUserName;

    // 数据库密码
    string databasePassword;

    // 数据库名
    string databaseName;

    // 日志写入方式
    int LOGWrite;

    // 触发组合模式
    int TRIGMode;

    // listenfd触发模式
    int LISTENTrigmode;

    // connfd触发模式
    int CONNTrigmode;

    // 优雅关闭链接
    int OPT_LINGER;

    // 数据库连接池数量
    int sql_num;

    // 线程池内的线程数量
    int thread_num;

    // 是否关闭日志
    int close_log;

    // 并发模型选择
    int actor_model;
};

#endif