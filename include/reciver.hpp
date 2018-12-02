#pragma once
#ifndef _LFTP_RDT_HPP_
#define _LFTP_RDT_HPP_

#include <winsock2.h>
#include <winuser.h>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdio>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include "packet.hpp"

#define DEFAULTRECIVERWINDOWSIZE 25
#define MAXWAITTIME 500
#define CONNECTIONMAXTIME 15000

using std::string;
using std::ifstream;
using std::ofstream;
using std::vector;
using std::thread;
using std::mutex;
using std::cout;
using std::ref;
using std::priority_queue;
using std::greater;

class RdtReciver {
  sockaddr_in localAddr;
  sockaddr_in remote;
  int sizeofScoketAddrIn;
  SOCKET socket;
  unsigned long windowSize; //窗口大小
  // unsigned long base;       //基序号
  unsigned long expectseqnum; //下一个包序号
  // mutex recv_send_mutex;
  char isTimeOut;
  char stopReciver;
  char shouldRestartTimer;
  ofstream out;
  priority_queue<unsigned long, vector<unsigned long>, greater<unsigned long>> buf;

 public:
  RdtReciver();
  ~RdtReciver();
  int rdt_rcv();
  void timer();
  int rdt_rcv_file(string filePath);
  int init(unsigned long targetIP, u_short port);
  void rdt_reciver_packets(Packet *packet, unsigned long n);
  SOCKET &getSocket();
  void restart();
};

#endif