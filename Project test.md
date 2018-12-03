# LFTP (C++) 测试文档
1. 在根目录下编译服务器:
   ```sh
   g++ lib\reciver.cpp lib\sender.cpp servers\servers.cpp -I include -lwsock32 -Wall -std=c++11 -o servers
   ```
   编译客户端:
   ```sh
   g++ lib\reciver.cpp lib\sender.cpp client\client.cpp -I include -lwsock32 -Wall -std=c++11 -o lftp
   ```
2. 