# network-chat-room [中文说明](README.md)

Related Conceptual References：

- [Wikipedia - network-chat-room](https://zh.m.wikipedia.org/wiki/%E7%B6%B2%E8%B7%AF%E8%81%8A%E5%A4%A9%E5%AE%A4 "Click to jump to the Wikipedia page")
- [Baidu Encyclopedia - network-chat-room](https://baike.baidu.com/item/%E7%BD%91%E7%BB%9C%E8%81%8A%E5%A4%A9%E5%AE%A4/2324705?_swebfr=220011 "Click to jump to the Baidu Encyclopedia page")

## Project brief introduction

I've put my life into creating an amazing space for communication in this fantastic online public chat room. With unmatched performance and efficient concurrent processing, this chat room makes every internet adventure silky smooth. Whether you're a passing visitor or a resident eager to share your feelings, this is the place to talk. Log in, sign up, and chat, all at your fingertips. By choosing to become a member, you will embark on a unique journey with exclusive privileges and experiences. In this vibrant and possible chat room, every exchange is an inspiring adventure, and every login is a fantastic encounter. It combines the power of technology and the warmth of humanity to create a digital home in the true sense, and we look forward to your joining us to write our wonderful story together.

## System architecture diagram

The main skeleton of the project is based on the non-blocking `Socket`+`Epoll` network listening system, uses the thread pool to process the sending and receiving of data with the client, uses the `MySQL` database connection pool to encapsulate multiple `MySQL` objects for data persistence management, uses a timer based on the ascending linked list to record the active state of the client to avoid waste of resources, and realizes the synchronous or asynchronous log system to record the running status of the server.

## Description of the project structure
>  network-chat-room  
>
> > `documents` -- Document resources such as environment construction, coding specifications, and project requirements
> > 
> > `server-cpp` -- `C++` Server project body
> >
> > `.gitignore` -- Ignore the commit configuration
> >
> > `LICENSE` -- `Apache` Permit
> > 
> > `README-CN.md` -- Project Readme - Chinese version
> > 
> > `README-EN.md` -- Project Readme - English version


## Environmental requirements

### Development tools

| tool            | illustrate                  | version      | remark                                                         |
| --------------- | --------------------- | --------- | ------------------------------------------------------------ |
| `DataGrip`      | Database connection tool        |  `2023.3.2`   | https://www.jetbrains.com/datagrip/cn/                        |
| `CMake`         | Cross-platform compilation tool       | `8.0.27`    |https://cmake.org/ |               |
| `Git`           | Project version control tool      | `1.8.3.1`    | https://git-scm.com/                                         |
| `GitHub`        | Project source code hosting platform      | `latest`    |https://github.com/                                    |
| `Axure`         | Prototyping tool          | `9`         | https://www.axure.com/                                       |
| `MindMaster`    | Mind map design tool      | `latest`    | http://www.edrawsoft.cn/mindmaster                           |
| `Visio`         | Flowchart drawing tool        | `latest`    | https://www.microsoft.com/zh-cn/microsoft-365/visio/flowchart-software |

### Server environment

| Dependent environment    | version                                                         | remark                                                         |
| ----------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| `CentOS` | `7.9`                                                      | https://www.centos.org/                               |
| `MySQL`     | `5.7.44`                                                       | https://www.mysql.com/cn/                                    |      |

## Special thanks to you

network-chat-room is inseparable from the support of open source software and the community, thanks to the following open source projects and project maintainers：

- `TinyWebServer`：https://github.com/qinguoyi/TinyWebServer.git
- `Linux`: https://github.com/torvalds/linux.git

We would also like to thank other open source components that have not been explicitly written out for the maintainers.

## Support it

If you think the framework and project are not bad, click on Star ⭐, which will be a great encouragement and support for Planet 01.

If you want to know more about computer direction selection, study advice and other related information, you can pay attention[**01 Planet bilibili station homepage~**](https://space.bilibili.com/1653229811?spm_id_from=333.1007.0.0)
