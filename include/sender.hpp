#pragma once
#ifndef _LFTP_RDT_SENDER_HPP_
#define _LFTP_RDT_SENDER_HPP_

#include <winsock2.h>
#include <winuser.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include "packet.hpp"

#define DEFAULTSENDERWINDOWSIZE 2
#define DEFAULTTIMEOUTINTERVAL 5000
#define MAXWINDOWSIZE 3000
#ifndef CONNECTIONMAXTIME
#define CONNECTIONMAXTIME 15000
#endif

using std::cout;
using std::ifstream;
using std::ofstream;
using std::ref;
using std::string;
using std::thread;
using std::vector;

class RdtSender {
  sockaddr_in targetAddr;
  sockaddr_in localAddr;
  int sizeofScoketAddrIn;
  SOCKET socket;
  double MaxTimeLimit;  //最长相应时间
  double EstimastedRTT;
  double DevRTT;
  unsigned long windowSize;  //窗口大小
  unsigned long base;        //基序号
  unsigned long nextseqnum;  //下一个包序号
  char isResend;
  char stopSender;
  ifstream in;
  char shouldRestartTimer;
  char counter;
  std::streampos fileSize;
  unsigned long cwnd;
  unsigned long ssthresh;
  enum Status { SLOWSTART = 0, CONGESTIONAVOIDANCE, FASTRECOVERY } status;
  enum Actions { NEWACK = 0, DUPACK, TIMEOUT };
  double helperCount;
  char isDisConnect;

  void timer();
  void CongestionControl(Actions act);
  int rdt_send();

 public:
  RdtSender();
  ~RdtSender();
  int rdt_rcv();
  int rdt_send_file(string filePath);
  int init(unsigned long targetIP, u_short port);
  void rdt_send_packets(Packet *packet, unsigned long n);
  sockaddr_in getRemote();
  void restart();
  SOCKET getSocket();
};

#endif