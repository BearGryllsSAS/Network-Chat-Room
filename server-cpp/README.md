# 工程介绍

项目主体骨架基于非阻塞 `Socket`+`Epoll` 网络监听体系，使用线程池处理与客户端数据的收发，采用 `MySQL` 数据库连接池
封装多个 `MySQL` 对象进行数据持久化管理，采用基于升序链表的定时器记录客户端的活跃状态以避免资源浪费，同时实现了同步/
异步日志系统记录服务器运行状态。

## 1 目录说明

- `chat` : 连接的用户信息目录
- `config` : 配置文件目录
- `lib-dy` ：动态库目录
- `lock` : 锁目录
- `log` ：日志文件目录
- `mysql` ：`mysql` 数据库目录
- `out` : 可执行程序输出目录
- `server` ：服务器主循环目录
- `threadpoll` ：线程池目录
- `timer` ：定时器目录
- `CMakeLists.txt` ：`cmake` 配置
- `main` : `main` 函数
- `README.md` ：工程自述文件

## 2 命名规范

- 目录名为小写，可用 `-` 连接
- 头文件、源文件采用大驼峰命名法
- 类名采用大驼峰命名法
- 函数名为小写，可以用 `_` 连接

## 3 启动测试

* 服务器测试环境

	* `CentOS` 版本 16.04
	* `MySQL` 版本 5.7.44
	* `CMake` 版本 8.0.27

* 测试前确认已安装 `MySQL` 数据库

    ```sql
    # 建立 server 数据库
    CREATE DATABASE server;

	# 使用 server 数据库
	USE server;

    # 创建 user 表
    CREATE TABLE user (
    	id INT AUTO_INCREMENT PRIMARY KEY,
    	usr_id INT NOT NULL,
    	usr_name VARCHAR(255) NOT NULL,
    	usr_password VARCHAR(255) NOT NULL
	);

    # 添加数据
    INSERT INTO user(usr_id, usr_name, usr_password) VALUES('12345', 'user', '123');
    ```

* 修改 `config/Config.cpp` 中的初始化信息

    ```C++
	// 服务器 ip 地址，默认172.27.169.171 
	serverIpAddr = "172.27.169.171";
	// 端口号，默认9006
	PORT = 9006;
	// 数据库 ip 地址，默认172.27.169.171
	databaseIpAddr = "172.27.169.171";
	// 数据库用户名，默认 root
	databaseUserName = "root";
	// 数据库密码，默认 123456
	databasePassword = "123456";
	// 数据库名，默认 server
	databaseName = "server";
    ```

* `CMake`
	* 在 `CMakeList.txt` 的工作目录下
    
	```C++
	// 创建 build 文件夹
	mkdir build
	// 进入 build 文件夹
	cd build
	// 执行 CMake 指令
	cmake ..
	// 执行 Makefile 指令
	make
	// 返回 CMakeList.txt 工作目录
	cd ..
    ```

* 启动server
	* 在 `CMakeList.txt` 的工作目录下

    ```C++
	// 启动服务器程序
    ./out/server
    ```

## 4 个性化运行

```C++
./out/server [-p port] [-l LOGWrite] [-m TRIGMode] [-o OPT_LINGER] [-s sql_num] [-t thread_num] [-c close_log] [-a actor_model] [-i serverIpAddr] [-I databaseIpAddr] [-U databaseUserName] [-P databasePassword] [-N databaseName]
```

温馨提示:以上参数不是非必须，不用全部使用，根据个人情况搭配选用即可.

* `-p`，自定义端口号
	* 默认9006

* `-l`，选择日志写入方式，默认同步写入
	* 0，同步写入
	* 1，异步写入

* `-m`，`listenfd` 和 `connfd` 的模式组合，默认使用 `LT` + `LT`
	* 0，表示使用 `LT` + `LT`
	* 1，表示使用 `LT` + `ET`
    * 2，表示使用 `ET` + `LT`
    * 3，表示使用 `ET` + `ET`

* `-o`，优雅关闭连接，默认不使用
	* 0，不使用
	* 1，使用

* `-s`，数据库连接数量
	* 默认为8

* `-t`，线程数量
	* 默认为8

* `-c`，关闭日志，默认打开
	* 0，打开日志
	* 1，关闭日志

* `-a`，选择反应堆模型，默认 `Proactor`
	* 0，`Proactor` 模型
	* 1，`Reactor` 模型

* -i，服务器 `ip` 地址

* -I，数据库 `ip` 地址

* -U，数据库用户名

* -P，数据库密码

* -N，数据库名

测试示例命令与含义

```C++
./out/server -p 9007 -l 1 -m 0 -o 1 -s 10 -t 10 -c 1 -a 1
```
- [x] 端口9007
- [x] 异步写入日志
- [x] 使用LT + LT组合
- [x] 使用优雅关闭连接
- [x] 数据库连接池内有10条连接
- [x] 线程池内有10条线程
- [x] 关闭日志
- [x] Reactor反应堆模型
