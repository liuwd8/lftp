#ifndef _LFTP_RDT_SENDER_HPP_
#define _LFTP_RDT_SENDER_HPP_

#include <winsock2.h>
#include <winuser.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include "packet.hpp"

#define DEFAULTWINDOWSIZE 2
#define DEFAULTTIMEOUTINTERVAL 5000
#define MAXWINDOWSIZE 3000

using std::string;
using std::ifstream;
using std::ofstream;
using std::vector;
using std::thread;
using std::mutex;
using std::cout;
using std::ref;

class RdtSender {
  sockaddr_in targetAddr;
  sockaddr_in localAddr;
  int sizeofScoketAddrIn;
  SOCKET socket;
  double MaxTimeLimit;         //最长相应时间
  double EstimastedRTT;
  double DevRTT;
  unsigned long windowSize; //窗口大小
  unsigned long base;       //基序号
  unsigned long nextseqnum; //下一个包序号
  mutex recv_send_mutex;
  char isResend;
  char stopSender;
  ifstream in;
  char shouldRestartTimer;
  char counter;
  std::streampos fileSize;
  unsigned long cwnd;
  unsigned long ssthresh;
  enum Status {SLOWSTART = 0, CONGESTIONAVOIDANCE, FASTRECOVERY} status;
  enum Actions {NEWACK = 0, DUPACK, TIMEOUT};
  double helperCount;
  
  void timer();
  void CongestionControl(Actions act);
 public:
  RdtSender();
  ~RdtSender();
  int rdt_send();
  int rdt_rcv();
  int rdt_send_file(string filePath);
  int init(unsigned long targetIP, u_short port);
};

#endif