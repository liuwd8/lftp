#include <winsock2.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include "sender.hpp"
#include "reciver.hpp"
using namespace std;

void recv(unsigned long targetIP, u_short port) {
  RdtReciver reciver;
  if (reciver.init(targetIP, port)) {
    cout << "some error happened.\n";
  }
}

int main(int argc, char const *argv[]) {
  string operate;
  string ipstr;
  string filePath;
  char ch;
  unsigned long ip;

  Packet packet;
  packet.header.seqnum = 0;

  if (argc > 0) {
    operate = string(argv[0]);
  } else {
    cout << "Hello, welcome to use lftp client.\n";
    cout << "Please input 'lsend' or 'lget'\n";
    cin >> operate;
  }
  
  if (operate._Equal("lget")) {
    ch = 'g';
  } else if (operate._Equal("lsend")) {
    ch = 's';
  } else {
    cout << operate << "is wrong parameter. please use lget or lsend.\n";
    return 0;
  }

  if (argc > 1) {
    ip = inet_addr(argv[1]);
    if (ip == INADDR_NONE) {
      cout << "Invalid ip address\n";
      return 0;
    }
  } else {
    cout << "Please input servers ip\n";
    while(cin >> ipstr) {
      if ((ip = inet_addr(ipstr.c_str())) != INADDR_NONE) {
        break;
      } else {
        cout << "Invalid ip address\n";
      }
    }
  }
  int count = 0;
  if (argc > 2) {
    ifstream in(argv[2]);
    if (!in) {
      cout << "file doesn't exit.\n";
      return 0;
    }
    in.close();
  } else {
    cout << "please input file path.\n";
    while(cin >> filePath) {
      ifstream in(filePath);
      if (!in) {
        cout << "file doesn't exit or file path is invalid.\n";
        count++;
      }
      if (count >= 3) {
        return 0;
      }
    }
  }

  WSADATA data;
  WORD sockVersion = MAKEWORD(2, 2);
  if (WSAStartup(sockVersion, &data) != 0) {
    cout << "WSAStartup failed!\n";
    return 0;
  }

  SOCKET socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (socket == INVALID_SOCKET) {
    cout << "socket create failed!\n";
    return 2;
  }
  sockaddr_in targetAddr, remote;
  targetAddr.sin_family = AF_INET;
  targetAddr.sin_port = htons(18123);
  targetAddr.sin_addr.S_un.S_addr = ip;

  int len = sizeof(sockaddr_in), ret;
  char buf[255] = {0};

  sprintf_s(packet.data, PACKETDATASIZE, "%c%s", ch, filePath.c_str());

  while (true) {
    sendto(socket, (char *)&packet, filePath.length + 1, 0, (sockaddr *)&targetAddr, len);
    ret = recvfrom(socket, buf, 255, 0, (sockaddr *)&remote, &len);
    if (ret > 0 && strcmp(packet.data, buf) == 0) {
      break;
    }
  }
  sendto(socket, filePath.c_str(), filePath.length, 0, (sockaddr *)&targetAddr, len);
  closesocket(socket);
  WSACleanup();
  return 0;
}
