#include "reciver.hpp"

RdtReciver::RdtReciver() {
  windowSize = DEFAULTRECIVERWINDOWSIZE;
  sizeofScoketAddrIn = sizeof(sockaddr_in);
  expectseqnum = 0;
  stopReciver = 0;
}

RdtReciver::~RdtReciver() {
  out.close();
  closesocket(socket);
  WSACleanup();
}

void RdtReciver::restart() { expectseqnum = 0; }

int RdtReciver::init(unsigned long targetIP, u_short port) {
  WSADATA data;
  WORD sockVersion = MAKEWORD(2, 2);
  if (WSAStartup(sockVersion, &data) != 0) {
    return 1;
  }
  socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  localAddr.sin_family = AF_INET;
  localAddr.sin_port = htons(port);
  localAddr.sin_addr.S_un.S_addr = targetIP;

  if (bind(socket, (sockaddr *)&localAddr, sizeofScoketAddrIn) ==
      SOCKET_ERROR) {
    return 2;
  }
  return 0;
}

int RdtReciver::rdt_rcv_file(string filePath) {
  out.open(filePath, std::ios::out | std::ios::binary);
  if (!out) {
    return 0;
  }
  thread recvTimer(&RdtReciver::timer, this);
  recvTimer.detach();
  return rdt_rcv();
}

int RdtReciver::rdt_rcv() {
  Packet packet;
  int ret;
  packet.header.fin = 0;
  while (expectseqnum * PACKETDATASIZE < packet.header.fileSize) {
    ret = recvfrom(socket, (char *)&packet, sizeof(packet), 0,
                   (sockaddr *)&remote, &sizeofScoketAddrIn);
    cout << "size: " << ret << ", seq: " << packet.header.seqnum << '\n';
    cout << "file size: " << (int)packet.header.fileSize
         << " sendTime :" << packet.header.sendTime << '\n';
    if (ret > 0) {
      packet.header.remaindSize = windowSize - buf.size();
      if (expectseqnum == packet.header.seqnum) {
        shouldRestartTimer = 1;
        out.seekp(packet.header.seqnum * PACKETDATASIZE);
        out.write(packet.data, packet.header.length);
        expectseqnum = expectseqnum + 1;
        while (!buf.empty()) {
          if (buf.top() > expectseqnum) {
            break;
          } else if (buf.top() == expectseqnum) {
            expectseqnum = expectseqnum + 1;
          }
          buf.pop();
        }
        packet.header.seqnum = expectseqnum;
        sendto(socket, (char *)&packet, sizeof(packetHeader), 0,
               (sockaddr *)&remote, sizeofScoketAddrIn);
      } else if (expectseqnum < packet.header.seqnum && buf.size() < windowSize) {
        out.seekp(packet.header.seqnum * PACKETDATASIZE);
        out.write(packet.data, packet.header.length);
        buf.push(packet.header.seqnum);
      }
    }
  }
  cout << "file recive success.\n";
  stopReciver = 1;
  expectseqnum = 0;
  rdt_reciver_packets(&packet, 1);
  return ret;
}

void RdtReciver::timer() {
  clock_t startTime = clock();
  clock_t secondPassed;
  int count = 0;
  cout << "timer start\n";
  while (!stopReciver) {
    secondPassed = clock() - startTime;
    if (secondPassed >= MAXWAITTIME) {
      if (expectseqnum) {
        cout << "time out\n";
        Packet packet;
        packet.header.seqnum = expectseqnum;
        packet.header.fin = 0;
        packet.header.length = 0;
        packet.header.remaindSize = windowSize - buf.size();
        packet.header.sendTime = clock_t();
        sendto(socket, (char *)&packet, sizeof(packetHeader), 0,
               (sockaddr *)&remote, sizeofScoketAddrIn);
      }
      shouldRestartTimer = 1;
      count = count + 1;
      startTime = clock();
    } else if (shouldRestartTimer) {
      cout << "restart time\n";
      startTime = clock();
      shouldRestartTimer = 0;
      count = 0;
    }
    if (count >= CONNECTIONMAXTIME / MAXWAITTIME) {
      cout << "connection time out.\n";
      this->~RdtReciver();
      break;
    }
  }
  cout << "timer end\n";
}

void RdtReciver::rdt_reciver_packets(Packet *packets, unsigned long n) {
  thread recvTimer(&RdtReciver::timer, this);
  recvTimer.detach();
  int ret;
  Packet packet;
  packet.header.fin = 0;
  while (expectseqnum < n && !packet.header.fin) {
    ret = recvfrom(socket, (char *)&packet, sizeof(packet), 0,
                   (sockaddr *)&remote, &sizeofScoketAddrIn);
    cout << "size: " << ret << ", seq: " << packet.header.seqnum << '\n';
    cout << "file size: " << (int)packet.header.fileSize
         << " sendTime :" << packet.header.sendTime << '\n';
    if (ret > 0) {
      packet.header.remaindSize = windowSize - buf.size();
      if (expectseqnum == packet.header.seqnum) {
        shouldRestartTimer = 1;
        snprintf(packets[packet.header.seqnum].data, packet.header.length, "%s",
                 packet.data);
        expectseqnum = expectseqnum + 1;
        while (!buf.empty()) {
          if (buf.top() != expectseqnum) {
            break;
          }
          buf.pop();
          expectseqnum = expectseqnum + 1;
        }
        packet.header.seqnum = expectseqnum;
        sendto(socket, (char *)&packet, sizeof(packetHeader), 0,
               (sockaddr *)&remote, sizeofScoketAddrIn);
      } else if (buf.size() < windowSize) {
        snprintf(packets[packet.header.seqnum].data, packet.header.length, "%s",
                 packet.data);
        buf.push(packet.header.seqnum);
      }
    }
  }
  stopReciver = 1;
}

SOCKET &RdtReciver::getSocket() { return socket; }