#include "ChatConn.h"
#include "../server/Server.h"

#include <algorithm>
#include <vector>
#include <set>
#include <mysql/mysql.h>
#include <fstream>
#include <ctime>

const char ChatConn::ms1[] = "与服务器建立连接, 开始进行数据通信 ------ [OK]\n"
             "          epoll服务器聊天室测试版          \n"
             "    (1)匿名聊天   (2)登陆   (3) 注册        \n"
             "========>";

Locker m_lock;
map<string, pair<string, string> > users;           //通过id映射用户名和密码
vector<int> onlineUsers;                            //记录在线用户，存放的是cfd
set<string> onlineUsersId;                          //记录在线用户，存放的是用户id。方便login函数判断

int ChatConn::m_user_count = 0;
int ChatConn::m_epollfd = -1;

//初始化连接
void ChatConn::initmysql_result(ConnectionPool *connPool)
{
    //先从连接池中取一个连接
    MYSQL *mysql = NULL;
    connectionRAII mysqlcon(&mysql, connPool);

    //在user表中检索username，passwd数据，浏览器端输入
    if (mysql_query(mysql, "SELECT usr_id, usr_name, usr_password FROM user"))
    {
        LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
    }

    //从表中检索完整的结果集
    MYSQL_RES *result = mysql_store_result(mysql);

    //从结果集中获取下一行，将对应的用户名和密码，存入map中
    while (MYSQL_ROW row = mysql_fetch_row(result))
    {
        string temp1(row[0]);
        string temp2(row[1]);
        string temp3(row[2]);

        users[temp1] = {temp2, temp3};
    }
}

//对文件描述符设置非阻塞
int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

//将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;
    
    if (1 == TRIGMode)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

//从内核时间表删除描述符
void removefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

//将事件重置为EPOLLONESHOT
void modfd(int epollfd, int fd, int ev, int TRIGMode)
{

    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    else
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP;

    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

//关闭连接，关闭一个连接，客户总量减一
void ChatConn::close_conn(bool real_close)
{
    if (real_close && (m_sockfd != -1))
    {
        printf("close %d\n", m_sockfd);
        removefd(m_epollfd, m_sockfd);

        // 以后更改为日志操作
        
        m_sockfd = -1;
        m_user_count--;
    }
}

//初始化连接,外部调用初始化套接字地址
void ChatConn::init(int sockfd, const sockaddr_in &addr, char *root, int TRIGMode,
                     int close_log, string user, string passwd, string sqlname)
{
    m_sockfd = sockfd;
    m_address = addr;

    this->fd = sockfd;

    // 添加监听事件
    addfd(m_epollfd, sockfd, true, m_TRIGMode);    //这里有点问题？？？最后一个参数还初始化呢

    //当浏览器出现连接重置时，可能是网站根目录出错或http响应格式出错或者访问的文件中内容完全为空
    m_TRIGMode = TRIGMode;
    m_close_log = close_log;

    strcpy(sql_user, user.c_str());
    strcpy(sql_passwd, passwd.c_str());
    strcpy(sql_name, sqlname.c_str());

    init();

    // 添加日志 -- 待定

    // 发送信息提示用户
    write(this->fd, ms1, sizeof ms1);
}

//初始化新接受的连接
//check_state默认为分析请求行状态
void ChatConn::init()
{
    mysql = NULL;
    timer_flag = 0;
    improv = 0;

    cb_fun = 0;

    memset(this->buf, '\0', BUFFER_SIZE);
}

//循环读取客户数据，直到无数据可读或对方关闭连接
//非阻塞ET工作模式下，需要一次性将数据读完
bool ChatConn::read_once()
{
    memset(this->buf, '\0', sizeof(this->buf));

    m_read_idx = 0;

    int bytes_read = 0;

    //LT读取数据
    if (0 == m_TRIGMode)
    {
        bytes_read = recv(m_sockfd, buf + m_read_idx, BUFFER_SIZE - m_read_idx, 0);
        m_read_idx += bytes_read;

        if (bytes_read <= 0)
        {
            return false;
        }

        this->buf[this->m_read_idx - 1] = '\0';

        return true;
    }
    //ET读数据
    else
    {
        while (true)
        {
            bytes_read = recv(m_sockfd, buf + m_read_idx, BUFFER_SIZE - m_read_idx, 0);
            if (bytes_read == -1)
            {
                this->buf[this->m_read_idx - 1] = '\0';

                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                return false;
            }
            else if (bytes_read == 0)
            {
                return false;
            }
            m_read_idx += bytes_read;
        }
        return true;
    }
}

// 出错处理函数
// 以后这些都会做一个封装，做成动态库
void sys_error(const char *str) 
{
    perror(str);
    exit;
}

// 登陆界面
void ChatConn::login_menu()
{
    if(this->buf[0] == '1')           // 匿名用户登陆
    {
        sprintf(this->usr_name, "匿名用户 %ld", time(NULL)) ;    // 设置匿名登陆的用户名
        strcpy(this->usr_id, "00000");                           // 所有匿名用户的账号为00000
        // 加入到聊天回调然后监听 
        this->log_step = 3;       // 表示为已登录状态

        // list_push(cfd);         // 加入当前的在线列表
        onlineUsers.push_back(this->fd);
        ChatConn::m_user_count++;        // 在线人数加一

        sprintf(this->buf,">               用户: %s  已登录,当前在线人数为 %d          \n\n>>>", this->usr_name, ChatConn::m_user_count);
        this->len = strlen(this->buf);
        char s[] = "----------------------epoll聊天室测试版--------------------\n";
        write(this->fd, s, strlen(s));
        write(this->fd, this->buf, this->len);

        this->cb_fun = 3;

        modfd(ChatConn::m_epollfd, this->fd, EPOLLOUT, this->m_TRIGMode);
    }
    else if(this->buf[0] == '2')          // 账号UID登陆
    {
        this->log_step = 1;
        strcpy(this->buf, "请输入登陆的UID:");
        write(this->fd, this->buf, strlen(this->buf));

        this->cb_fun = 1;

        modfd(ChatConn::m_epollfd, this->fd, EPOLLIN, this->m_TRIGMode);
    }
    else   // 注册
    {
        strcpy(this->buf, "注册账号\n###请输入注册的用户名(中文/英文, 注意不要包含特殊符号): ");
        write(this->fd, this->buf, strlen(this->buf));
        this->log_step = 4;           // 标记为进行注册状态的准备输入注册的用户名

        this->cb_fun = 2;

        modfd(ChatConn::m_epollfd, this->fd, EPOLLIN, this->m_TRIGMode);
    }

}

// 输入账号UID进行登陆
void ChatConn::login()
{
    if(1 == this->log_step)     // 读取用户输入用户名
    {
        int id = atoi(this->buf);
        strcpy(this->usr_id, this->buf);
        char s[100];
        if(users.find(to_string(id)) == users.end())      
        {
            sprintf(s, "!用户UID:%s 不存在\n请重新输入账号UID:", this->buf);
            write(this->fd, s, strlen(s));
            modfd(ChatConn::m_epollfd, this->fd, EPOLLIN, this->m_TRIGMode);
            return;
        }
        if(onlineUsersId.find(to_string(id)) != onlineUsersId.end())                      
        {
            sprintf(s, "!用户UID:%s 已登陆\n请重新输入账号UID:", this->buf);
            write(this->fd, s, strlen(s));
            modfd(ChatConn::m_epollfd, this->fd, EPOLLIN, this->m_TRIGMode);
            return;
        }
        strcpy(this->buf, "请输入密码:");
        write(this->fd, this->buf, strlen(this->buf));

        memset(this->buf, '\0', sizeof(this->buf));

        modfd(ChatConn::m_epollfd, this->fd, EPOLLIN, this->m_TRIGMode);

    }
    else if(2 == this->log_step)  // 输入用户密码
    {
        int id = atoi(this->usr_id);
        strcpy(this->usr_key, this->buf);

        string temp(this->buf);

        if(temp.compare(users[to_string(id)].second) == 0)
        {
            strcpy(this->usr_name, users[to_string(id)].first.c_str());  //数据库中还是得记录用户名，并且初始化的时候还是得把用户名读出来

            // list_push(cfd);                                 // 将当前的cfd添加进在线列表中
            onlineUsers.emplace_back(this->fd);         

            // Users[id].st = 1;
            onlineUsersId.insert(this->usr_id);

            //在线人数加一
            ChatConn::m_user_count++;
            
            sprintf(this->buf,">               用户: %s  已登录,当前在线人数为 %d          \n\n>>>", this->usr_name, ChatConn::m_user_count);
            this->len = strlen(this->buf);
            char s[] = "----------------------epoll聊天室测试版--------------------\n";
            write(this->fd, s, sizeof s);
            write(this->fd, this->buf, this->len);

            this->cb_fun = 3;

            modfd(ChatConn::m_epollfd, this->fd, EPOLLOUT, this->m_TRIGMode);
        }
        else
        {
            strcpy(this->buf, "密码错误, 请重新输入密码:");
            write(this->fd, this->buf, strlen(this->buf));

            bzero(this->buf, sizeof(this->buf));
            modfd(ChatConn::m_epollfd, this->fd, EPOLLIN, this->m_TRIGMode);

            return;
        }

    }
    this->log_step++;
}

// 注册账号
void ChatConn::register_id()
{
    if(4 == this->log_step)     // 输入注册的用户名
    {
        strcpy(this->usr_name, this->buf);
        strcpy(this->buf, "请设定账号的密码: ");
        write(this->fd, this->buf, strlen(this->buf));
    }
    else if(5 == this->log_step)  // 输入用户密码
    {
        strcpy(this->usr_key, this->buf);
        strcpy(this->buf, "请再次输入密码: ");
        write(this->fd, this->buf, strlen(this->buf));
    }
    else if(6 == this->log_step)  // 验证两次用户的密码
    {
        if(strcmp(this->usr_key, this->buf))
        { 
            strcpy(this->buf, "两次密码输入不一致, 请重新输入:");
            write(this->fd, this->buf, strlen(this->buf));
            return;
        }
        this->get_uid();
        sprintf(this->buf, "注册成功, 你的账号uid: %s  用户名为%s, 现在重新返回登陆界面 \n\n", this->usr_id,this->usr_name);
  
        this->log_step = 0;
        write(this->fd, this->buf, strlen(this->buf));
        write(this->fd, ms1, sizeof ms1);

        this->cb_fun = 0;

        modfd(ChatConn::m_epollfd, this->fd, EPOLLIN, this->m_TRIGMode);

        return;
    }
    
    modfd(ChatConn::m_epollfd, this->fd, EPOLLIN, this->m_TRIGMode);
    
    this->log_step++;
}

// 获取一个未注册的uid
void ChatConn::get_uid()
{
    srand(static_cast<unsigned int>(time(nullptr)));

    int num = 0;
    do
    {
        num = rand() % 90000 + 10000;

    } while(users.find(to_string(num)) != users.end());


    char str[10];
    sprintf(str, "%05d", num);   
    strcpy(this->usr_id, str); 

    //将注册的用户信息加入users中 将注册的用户信息加入数据库中
    char *sql_insert = (char *)malloc(sizeof(char) * 200);
    strcpy(sql_insert, "INSERT INTO user(usr_id, usr_name, usr_password) VALUES(");
    strcat(sql_insert, "'");
    strcat(sql_insert, this->usr_id);
    strcat(sql_insert, "', '");
    strcat(sql_insert, this->usr_name);
    strcat(sql_insert, "', '");
    strcat(sql_insert, this->usr_key);
    strcat(sql_insert, "')");
   
    string temp1(this->usr_id);
    string temp2(this->usr_name);
    string temp3(this->usr_key);

    m_lock.lock();
    int res = mysql_query(mysql, sql_insert);
    users[temp1] = {temp2, temp3};
    m_lock.unlock();

    for (auto user : users)
    {
        cout << "id = " << user.first << ' ' << "name = " << user.second.first << ' ' << "password = " << user.second.second << endl;
    }
}

// 写事件 ---> 向当前在线用户发送信息
bool ChatConn::cb_write()
{   

    if (this->len <= 0) return false;

    for(const auto& onlinefd : onlineUsers)             // 遍历当前的在线链表, 向在线用户发送
    {
        if(onlinefd == this->fd) continue;           // 发送数据给服务器的客户端一方并不需要发送
        write(onlinefd, this->buf, this->len);
    }

    if(this->log_step == 3) write(this->fd, "\n>>>", 4);   // 界面的优化,与主要逻辑无关 

    if(this->log_step == 0) return false;   // 状态为登出


    bzero(this->buf, sizeof(this->buf));

    this->cb_fun = 4;

    modfd(ChatConn::m_epollfd, this->fd, EPOLLIN, this->m_TRIGMode);

    return true;
}

// 读事件 -----> 服务器接收的客户端发来的信息
void ChatConn::cb_read()
{
    char str[BUFSIZ], str2[1024];

    strcpy(str, this->buf); 
    
    //不确定是否否要用
    memset(this->buf, '\0', sizeof(this->buf));
    
    //将来这一步要录入日志文件中
    sprintf(str2, "from client fd: %d receive data is :", this->fd);
    // if(ret > 0)  write(STDOUT_FILENO, str2, strlen(str2));
    // write(STDOUT_FILENO, str2, strlen(str2));

    // cout << "in cb_read str is " << str << "   " << "strlen(str) is " << strlen(str) << endl; 
 
    // write(STDOUT_FILENO, str, strlen(str));    // 将客户端发来的数据在服务器端进行打印

    sprintf(this->buf, "(%s):%s\n>>>", this->usr_name, str);   // 格式化客户端发来的数据 --- 数据处理
    this->len = strlen(this->buf);


    modfd(ChatConn::m_epollfd, this->fd, EPOLLOUT, this->m_TRIGMode);
}

// 登出操作 ---> 必须是登陆上之后进行登出才调用
//这里面要统一在timer那里处理，因为数据是在外面读取的
void ChatConn::logout()
{        
    char str[1024];  
    this->log_step = 0;   // 标记为登出                    
   
    m_lock.lock();   //处理逻辑待定
    onlineUsers.erase(std::remove(onlineUsers.begin(), onlineUsers.end(), this->fd), onlineUsers.end());                  // 从在线列表中删除                      
    onlineUsersId.erase(this->usr_id);   // 用户信息中将其标记为离线状态
    ChatConn::m_user_count--;
    m_lock.unlock();     

    sprintf(str, "已退出聊天室, 当前在线人数为%d\n", ChatConn::m_user_count);
    sprintf(this->buf, "(%s) %s\n>>>", this->usr_name, str);
    this->len = strlen(this->buf);
    cb_write();                  // 手动调用向其他用户发送XXX用户登出的信息
}



void ChatConn::process()
{
    // 调用回调函数
    // this->fun();
    switch(this->cb_fun)
    {
        case 0:
        {
            this->login_menu();
            break;
        }
        case 1:
        {
            this->login();
            break;
        }
        case 2:
        {
            this->register_id();
            break;
        }
        case 3:
        {
            this->cb_write();
            break;
        }
        case 4:
        {
            this->cb_read();
            break;
        }
        default:
        {
             break;
        }   
    }

    return;
}