#include "sender.hpp"

RdtSender::RdtSender() {
  base = 0;
  nextseqnum = 0;
  windowSize = DEFAULTWINDOWSIZE;
  sizeofScoketAddrIn = sizeof(sockaddr_in);
  MaxTimeLimit = DEFAULTTIMEOUTINTERVAL;
  EstimastedRTT = 500;
  DevRTT = 0;
  isResend = 0;
  shouldRestartTimer = 0;
  stopSender = 0;
  counter = 0;
  fileSize = 0;
  status = SLOWSTART;
  ssthresh = 64;
  cwnd = 1;
  helperCount = 0;
}

RdtSender::~RdtSender() {
  in.close();
  closesocket(socket);
  WSACleanup();
}

int RdtSender::init(unsigned long targetIP, u_short port) {
  WSADATA data;
  WORD sockVersion = MAKEWORD(2, 2);
  if (WSAStartup(sockVersion, &data) != 0) {
    return 1;
  }
  socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (socket == INVALID_SOCKET) {
    return 2;
  }
  targetAddr.sin_family = AF_INET;
  targetAddr.sin_port = htons(port);
  targetAddr.sin_addr.S_un.S_addr = targetIP;

  localAddr.sin_family = AF_INET;
  localAddr.sin_port = htons(0);
  localAddr.sin_addr.S_un.S_addr = INADDR_ANY;
  
  if (bind(socket, (sockaddr *)&localAddr, sizeofScoketAddrIn) == SOCKET_ERROR) {
    return 3;
  }
  
  return 0;
}

int RdtSender::rdt_send_file(string filePath) {
  in.open(filePath, std::ios::in|std::ios::binary);
  if (!in) {
    return 1;
  }
  in.seekg(0, in.end);
  fileSize = in.tellg();
  in.seekg(0, in.beg);
  thread recvThread(&RdtSender::rdt_rcv, this);
  thread timerThread(&RdtSender::timer, this);
  recvThread.detach();
  timerThread.detach();
  return rdt_send();
}
int RdtSender::rdt_send() {
  Packet packet;
  packet.header.fin = 0;
  packet.header.fileSize = fileSize;
  while(base * PACKETDATASIZE < fileSize) {
    if (isResend) {
      isResend = 0;
      nextseqnum = base;
      in.seekg(nextseqnum * PACKETDATASIZE);
    }
    if (!in.eof() && nextseqnum < base + windowSize) {
      in.read(packet.data, PACKETDATASIZE);
      packet.header.seqnum = nextseqnum;
      packet.header.length = in.gcount();
      packet.header.sendTime = clock();
      cout << "send packet: " << packet.header.seqnum << '\n';
      ::sendto(socket,(char *)&packet, packet.header.length + sizeof(packetHeader), 0,(sockaddr *)&targetAddr, sizeof(targetAddr));
      if (base == nextseqnum) {
        shouldRestartTimer = 1;
      }
      nextseqnum = nextseqnum + 1;
    }
  }
  stopSender = 1;
  cout << "send complete!\n";
  return 0;
}

void RdtSender::timer() {
  clock_t startTime = clock();
  double secondPassed;
  double second;
  cout << "timer start\n";
  while (!stopSender) {
    secondPassed = clock() - startTime;
    second = clock() - second;
    if (!isResend && secondPassed >= MaxTimeLimit) {
      cout << "time out\n";
      isResend = 1;
      MaxTimeLimit = MaxTimeLimit * 2;
      CongestionControl(TIMEOUT);
    } else if (shouldRestartTimer) {
      cout << "restart time\n";
      startTime = clock();
      shouldRestartTimer = 0;
    }
    if (second >= 1000) {
      cout << "speed: " << base * 1000.0 / clock() << " kb/s\n";
      second = clock();
    }
  }
  cout << "timer end\n";
}

int RdtSender::rdt_rcv() {
  Packet packet;
  sockaddr_in remote;
  int ret;
  while (!stopSender) {
    ret = recvfrom(socket, (char *)&packet, sizeof(packet), 0,(sockaddr *)&remote, &sizeofScoketAddrIn);
    clock_t SampleRTT = clock() - packet.header.sendTime;
    if (ret > 0 && packet.header.seqnum > base) {
      cout << "should restart timer\n" << "base to:" << packet.header.seqnum <<'\n';
      shouldRestartTimer = 1;
      base = packet.header.seqnum;
      CongestionControl(NEWACK);
    } else {
      SampleRTT = SampleRTT << 1;
      cout << "seq: " << packet.header.seqnum << " is ack!\n";
      CongestionControl(DUPACK);
      if (counter > 2) {
        base = packet.header.seqnum;
        isResend = 1;
      }
    }
    windowSize = ((packet.header.remaindSize ^ cwnd) & -(packet.header.remaindSize < cwnd)) ^ cwnd;
    if (windowSize == 0) {
      windowSize = 1;
    }
    cout << "windows size: " << windowSize << '\n';
    cout << "SampleRTT: " << SampleRTT << '\n';
    EstimastedRTT = 0.875 * EstimastedRTT + 0.125 * SampleRTT;
    DevRTT = 0.75 * DevRTT + 0.25 * abs(EstimastedRTT - SampleRTT);
    MaxTimeLimit = EstimastedRTT + 4 * DevRTT;
  }
  return ret;
}

void RdtSender::CongestionControl(Actions act) {
  switch (status) {
    case SLOWSTART: //慢启动
      if (act == NEWACK) {
        cwnd = cwnd + 1;
        counter = 0;
      } else if (act == TIMEOUT) {
        ssthresh = cwnd >> 1;
        cwnd = 1;
        counter = 0;
      } else if (act == DUPACK) {
        counter = counter + 1;
      }
      if (counter > 2) {
        ssthresh = cwnd / 2;
        cwnd = ssthresh + 3;
        status = FASTRECOVERY;
      } else if (cwnd >= ssthresh) {
        status = CONGESTIONAVOIDANCE;
      }
      break;
    case CONGESTIONAVOIDANCE: //拥塞避免
      if (act == NEWACK) {
        helperCount = helperCount + 1.0/cwnd;
        cwnd = cwnd + (long) helperCount;
        counter = 0;
      } else if (act == TIMEOUT) {
        ssthresh = cwnd / 2;
        cwnd = 1;
        status = SLOWSTART;
        counter = 0;
      } else if (act == DUPACK) {
        counter = counter + 1;
      }
      if (counter > 2) {
        ssthresh = cwnd / 2;
        cwnd = ssthresh + 3;
        status = FASTRECOVERY;
      }
      break;
    case FASTRECOVERY: //快速恢复
      if (act == NEWACK) {
        cwnd = ssthresh;
        counter = 0;
        status = CONGESTIONAVOIDANCE;
      } else if (act == TIMEOUT) {
        ssthresh = cwnd / 2;
        cwnd = 1;
        status = SLOWSTART;
        counter = 0;
      } else if (act == DUPACK) {
        cwnd = cwnd + 1;
      }
  }
}

void RdtSender::rdt_send_packet(Packet *packets, int n) {
  thread recvThread(&RdtSender::rdt_rcv, this);
  thread timerThread(&RdtSender::timer, this);
  recvThread.detach();
  timerThread.detach();
  base = 0;
  nextseqnum = 0;
  windowSize = 1;
  while (base < n) {
    if (isResend) {
      isResend = 0;
      nextseqnum = base;
    }
    if (nextseqnum < n && nextseqnum < base + windowSize) {
      packets[base].header.seqnum = nextseqnum;
      packets[base].header.length = in.gcount();
      packets[base].header.sendTime = clock();
      cout << "send packet: " << packets[base].header.seqnum << '\n';
      ::sendto(socket,(char *)&packets[base], packets[base].header.length + sizeof(packetHeader), 0,(sockaddr *)&targetAddr, sizeof(targetAddr));
      if (base == nextseqnum) {
        shouldRestartTimer = 1;
      }
      nextseqnum = nextseqnum + 1;
    }
  }
  stopSender = 1;
  cout << "send complete!\n";
}