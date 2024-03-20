#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <map>
#include <functional>

#include "../lock/Locker.h"
#include "../mysql/SqlConnectionPool.h"
#include "../timer/ListTimer.h"
#include "../log/Log.h"

// typedef void (*call_back)(); 

class ChatConn
{
public:
    int cb_fun;

    static const char ms1[];
     long m_read_idx;
     static const int BUFFER_SIZE = 2048;

public:
    ChatConn() {}
    ~ChatConn() {}

public:
    void init(int sockfd, const sockaddr_in &addr, char *, int, int, string user, string passwd, string sqlname);
    void close_conn(bool real_close = true);
    void process();
    bool read_once();
    sockaddr_in *get_address()
    {
        return &m_address;
    }
    void initmysql_result(ConnectionPool *connPool);
    int timer_flag;
    int improv;


private:
    void init();

public:
    static int m_epollfd;
    static int m_user_count;
    MYSQL *mysql;
    int m_state;  //读为0, 写为1

private:
    int m_sockfd;
    sockaddr_in m_address;

    map<string, string> m_users;
    int m_TRIGMode;
    int m_close_log;

    char sql_user[100];
    char sql_passwd[100];
    char sql_name[100];

public:
    int fd;                     // 监听的文件描述符
    char buf[BUFFER_SIZE];           // 读写缓冲区
    int len;                    // 本次从客户端读入缓冲区数据的长度
    int log_step;               // 标记用户位于登陆的操作 0-- 未登陆  1 --- 输入账号  2 ---- 输入密码   3----- 成功登陆  4 --- 注册用户名  5 ------ 输入注册的密码   6 ------- 再次输入密码验证

    char usr_id[8];            // 用户ID    五位 UID
    char usr_name[256];        // 用户名
    char usr_key[40];          // 用户密码

public:
    // 登陆界面
    void login_menu();

    // 输入账号UID进行登陆
    void login();

    // 注册账号
    void register_id();    

    // 写事件 ---> 向当前在线用户发送信息
    bool cb_write();

    // 读事件 -----> 服务器接收的客户端发来的信息
    void cb_read();

    // 登出操作 ---> 必须是登陆上之后进行登出才调用
    void logout();

    // 获取一个未注册的uid
    void get_uid();
};

#endif
