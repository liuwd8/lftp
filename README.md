# LFTP(c++, windows平台)
1. 基于`UDP`协议
2. rdt3.0 和流水线 rdt
3. 快速重传
4. 流控制
5. 拥塞控制
6. 服务端支持多客户端同时建立连接

## 设计报告

[设计报告](https://github.com/liuwd8/lftp/blob/master/Project%20Design.md)

## 测试报告

[测试报告](https://github.com/liuwd8/lftp/blob/master/Project%20test.md)

## 使用方法
客户端：
```sh
// 拉取文件
lftp lget 服务器ip 文件路径
// 上传文件
lftp lsend 服务器ip 文件路径
```
服务端:
```sh
servers
```
## 编译方法
根目录下使用以下方法编译得到客户端和服务端, `bin`文件夹中有已经编译好的。
```sh
g++ lib\reciver.cpp lib\sender.cpp servers\servers.cpp -I include -lwsock32 -Wall -std=c++11 -o servers
```

```sh
g++ lib\reciver.cpp lib\sender.cpp client\client.cpp -I include -lwsock32 -Wall -std=c++11 -o lftp
```

## 注意事项
* 服务器不会自动退出，除非使用<kbd>ctrl</kbd> + <kbd>c</kbd>
* 服务端和客户端使用的路径是相同的，服务端根据客户端发送的路径来创建或者选择文件。所以当服务端不存在文件，而客户端使用 `lget`命令时，无提示，也不会终止。但是其实是无法获取文件的。`lsend`同理。

## LICENSE
MIT
