# LFTP (C++) 测试文档
1. 在根目录下编译服务器:
   ```sh
   g++ lib\reciver.cpp lib\sender.cpp servers\servers.cpp -I include -lwsock32 -Wall -std=c++11 -o servers
   ```
   编译客户端:
   ```sh
   g++ lib\reciver.cpp lib\sender.cpp client\client.cpp -I include -lwsock32 -Wall -std=c++11 -o lftp
   ```
2. 远程传输文件测试
    * 首先是小文件：data.h
        * 客户端
            | lsend | lget |
            | :---: | :---: |
            |![](images/lftp-lsend.png)| ![](images/lftp-lget.png)|
        * 服务器端：
            ![](images/servers-little.png)
    * 大文件日志太长，日志信息保存在logs文件夹中。
