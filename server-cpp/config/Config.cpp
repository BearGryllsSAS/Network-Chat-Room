#include "Config.h"

Config::Config(){
    // 服务器 ip 地址，默认172.27.169.171 
    serverIpAddr = "172.27.169.171";

    // 端口号，默认9006
    PORT = 9006;

    // 数据库 ip 地址，默认172.27.169.171
    databaseIpAddr = "172.27.169.171";

    // 数据库用户名
    databaseUserName = "root";

    // 数据库密码
    databasePassword = "123456";

    // 数据库名
    databaseName = "server";

    // 日志写入方式，默认同步
    LOGWrite = 0;

    // 触发组合模式，默认 listenfd LT + connfd LT
    TRIGMode = 0;

    // listenfd 触发模式，默认 LT
    LISTENTrigmode = 0;

    // connfd 触发模式，默认 LT
    CONNTrigmode = 0;

    // 优雅关闭链接，默认不使用
    OPT_LINGER = 0;

    // 数据库连接池数量，默认8
    sql_num = 8;

    // 线程池内的线程数量，默认8
    thread_num = 8;

    // 关闭日志，默认不关闭
    close_log = 0;

    // 并发模型，默认是proactor
    actor_model = 0;
}

void Config::parse_arg(int argc, char*argv[]){
    int opt;
    const char *str = "p:l:m:o:s:t:c:a:i:I:U:P:N:";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
        case 'U':
        {
            databaseUserName = optarg;
            break;
        }
        case 'P':
        {
            databasePassword = optarg;
            break;
        }
        case 'N':
        {
            databaseName = optarg;
            break;
        }
        case 'I':
        {
            databaseIpAddr = optarg;
            break;
        }
        case 'i':
        {
            serverIpAddr = optarg;
            break;
        }
        case 'p':
        {
            PORT = atoi(optarg);
            break;
        }
        case 'l':
        {
            LOGWrite = atoi(optarg);
            break;
        }
        case 'm':
        {
            TRIGMode = atoi(optarg);
            break;
        }
        case 'o':
        {
            OPT_LINGER = atoi(optarg);
            break;
        }
        case 's':
        {
            sql_num = atoi(optarg);
            break;
        }
        case 't':
        {
            thread_num = atoi(optarg);
            break;
        }
        case 'c':
        {
            close_log = atoi(optarg);
            break;
        }
        case 'a':
        {
            actor_model = atoi(optarg);
            break;
        }
        default:
            break;
        }
    }
}