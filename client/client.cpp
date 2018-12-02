#include <winsock2.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include "sender.hpp"
#include "reciver.hpp"
using namespace std;

int main(int argc, char const *argv[]) {
  string operate;
  string ipstr;
  string filePath;
  char ch;
  unsigned long ip;

  if (argc > 1) {
    operate = string(argv[1]);
  } else {
    cout << "Hello, welcome to use lftp client.\n";
    cout << "Please input 'lsend' or 'lget'\n";
    cin >> operate;
  }
  const char *str = operate.c_str();
  if (strcmp(str,"lget") == 0) {
    ch = 'g';
  } else if (strcmp(str,"lsend") == 0) {
    ch = 's';
  } else {
    cout << operate << "is wrong parameter. please use lget or lsend.\n";
    return 0;
  }

  if (argc > 2) {
    ip = inet_addr(argv[2]);
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
  if (argc > 3) {
    filePath = string(argv[3]);
    if (ch == 's') {
      ifstream in(filePath, ios::in|ios::binary);
      if (!in) {
        cout << "file doesn't exit or file path is invalid.\n";
        return 0;
      } else {
        in.close();
      }
    } else if (ch == 'g') {
      ofstream out(filePath, ios::out|ios::binary);
      if (!out) {
        cout << "can't write file to " << filePath << " .\n";
      } else {
        out.close();
      }
    } else {
      cout << "unexpected input.\n";
      return 0;
    }
  } else {
    cout << "please input file path.\n";
    while(cin >> filePath) {
      if (ch == 's') {
        ifstream in(filePath, ios::in|ios::binary);
        if (!in) {
          cout << "file doesn't exit or file path is invalid.\n";
          count++;
        } else {
          in.close();
          break;
        }
        if (count >= 3) {
          return 0;
        }
      } else if (ch == 'g') {
        ofstream out(filePath, ios::out|ios::binary);
        if (!out) {
          cout << "can't write file to " << filePath << " .\n";
          count++;
        } else {
          out.close();
          break;
        }
        if (count >= 3) {
          return 0;
        }
      }
    }
  }

  RdtSender sender;
  if (sender.init(ip, 18888)) {
    cout << "init failed.\n";
    return 0;
  }
  Packet packets[1];
  if (ch == 's') {
    snprintf(packets[0].data, PACKETDATASIZE, "%c%s", ch, filePath.c_str());
    packets[0].header.length = (unsigned long) filePath.size() + 1;
    sender.rdt_send_packets(packets, 1);
    sender.restart();
    sender.rdt_send_file(filePath);
  } else if (ch == 'g') {
    snprintf(packets[0].data, PACKETDATASIZE, "%c%s", ch, filePath.c_str());
    packets[0].header.length = 1 + filePath.size();
    packets[0].header.seqnum = 0;
    packets[0].header.sendTime = clock();
    packets[0].header.fileSize = 0;
    int ret = 1;
    int length = 1 + filePath.size();
    int len = sizeof(sockaddr);
    sockaddr_in remote = sender.getRemote();
    Packet *p = &packets[0];
    while (true) {
      if (ret > 0 && packets[0].data[0] == 'g') {
        thread newReciverThread([=] {
          RdtReciver reciver;
          reciver.init(INADDR_ANY, 0);
          cout << "syn with server.\n";
          cout << "data" << packets[0].data << '\n';
          int len = 16;
          sockaddr_in back;
          getsockname(reciver.getSocket(), (sockaddr *)&back, &len);
          cout << "port " << back.sin_port << '\n';
          sendto(reciver.getSocket(), (char *)packets, sizeof(packetHeader) + length, 0, (sockaddr *)&remote, sizeof(sockaddr));
          reciver.rdt_reciver_packets(p, 1);
          cout << "syn is ack.\n";
          cout << "start recive.\n";
          reciver.restart();
          reciver.rdt_rcv_file(filePath);
          exit(0);
        });
        newReciverThread.detach();
      }
      ret = recvfrom(sender.getSocket(), (char *)packets, sizeof(Packet), 0, (sockaddr *)&remote, &len);
    }
  }
  return 0;
}
