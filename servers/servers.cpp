#include <winsock2.h>
#include <csignal>
#include <fstream>
#include <iostream>
#include "packet.hpp"
#include "reciver.hpp"
#include "sender.hpp"

SOCKET *s;
char isRunning = 1;
// 主线程仅用来接收数据，当合法的数据到来时，开始新的线程，在发送数据。
void handle(int x) {
  if (x == SIGINT) {
    cout << "bye!\n";
    isRunning = 0;
    closesocket(*s);
    WSACleanup();
    exit(0);
  }
}

int main(int argc, char const *argv[]) {
  WSADATA data;
  WORD sockVersion = MAKEWORD(2, 2);
  signal(SIGINT, handle);
  if (WSAStartup(sockVersion, &data) != 0) {
    return 1;
  }
  SOCKET socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  s = &socket;
  sockaddr_in localAddr;
  localAddr.sin_family = AF_INET;
  localAddr.sin_port = htons(18888);
  localAddr.sin_addr.S_un.S_addr = INADDR_ANY;

  if (bind(socket, (sockaddr *)&localAddr, sizeof(sockaddr_in)) ==
      SOCKET_ERROR) {
    cout << "bind error.\n";
    return 0;
  }
  int ret;
  sockaddr_in remote;
  Packet packet;
  int len = sizeof(sockaddr_in);
  while (isRunning) {
    cout << "main thread working" << std::endl;
    ret = recvfrom(socket, (char *)&packet, sizeof(Packet) - 1, 0,
                   (sockaddr *)&remote, &len);
    if (ret > 0) {
      Packet packetBackup;
      Packet *p = &packetBackup;
      packetBackup.header.remaindSize = DEFAULTRECIVERWINDOWSIZE;
      packetBackup.header.sendTime = packet.header.sendTime;
      packetBackup.header.fin = packet.header.fin;
      packetBackup.header.fileSize = packet.header.fileSize;
      packetBackup.header.seqnum = packet.header.seqnum + 1;
      packet.data[packet.header.length] = 0;
      string str = string(&packet.data[1]);
      char ch = packet.data[0];
      cout << ch << '\n';
      if (ch == 's') {
        cout << "recive " << str << '\n';
        thread newReciverThread([=] {
          RdtReciver reciver;
          reciver.init(INADDR_ANY, 0);
          sendto(reciver.getSocket(), (char *)p, sizeof(packetHeader), 0,
                 (sockaddr *)&remote, sizeof(sockaddr));
          reciver.rdt_rcv_file(str);
        });
        newReciverThread.detach();
      } else if (ch == 'g') {
        thread newSenderThread([=] {
          RdtSender sender;
          sender.init(remote.sin_addr.S_un.S_addr, htons(remote.sin_port));
          sender.rdt_send_packets(p, 1);
          cout << "recive the replay.\nstart to send file " << str << '\n';
          sender.restart();
          sender.rdt_send_file(str);
        });
        newSenderThread.detach();
      }
    }
  }
  return 0;
}
