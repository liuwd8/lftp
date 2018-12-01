#include "reciver.hpp"

RdtReciver::RdtReciver() {
  windowSize = DEFAULTWINDOWSIZE;
  sizeofScoketAddrIn = sizeof(sockaddr_in);
  expectseqnum = 0;
  stopReciver = 0;
}

RdtReciver::~RdtReciver() {
  out.close();
  closesocket(socket);
  WSACleanup();
}


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

  if (bind(socket, (sockaddr *)&localAddr, sizeofScoketAddrIn) == SOCKET_ERROR) {
    return 2;
  }
  return 0;
}

int RdtReciver::rdt_rcv_file(string filePath) {
  out.open(filePath, std::ios::out|std::ios::binary);
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
    ret = recvfrom(socket, (char *)&packet, sizeof(packet), 0,(sockaddr *)&remote, &sizeofScoketAddrIn);
    cout << "size: " << ret << ", seq: " << packet.header.seqnum << '\n';
    cout << "file size: " << (int)packet.header.fileSize << " sendTime :" << packet.header.sendTime << '\n';
    if (ret > 0) {
      packet.header.remaindSize = windowSize - buf.size();
      if (expectseqnum == packet.header.seqnum) {
        shouldRestartTimer = 1;
        out.seekp(packet.header.seqnum * PACKETDATASIZE);
        out.write(packet.data, packet.header.length);
        expectseqnum = expectseqnum + 1;
        while (!buf.empty()) {
          if (buf.top() != expectseqnum) {
            break;
          }
          buf.pop();
          expectseqnum = expectseqnum + 1;
        }
        packet.header.seqnum = expectseqnum;
        sendto(socket, (char *)&packet, sizeof(packetHeader), 0, (sockaddr *)&remote, sizeofScoketAddrIn);
      } else if (packet.header.seqnum < expectseqnum) {
        packet.header.seqnum = expectseqnum;
        sendto(socket, (char *)&packet, sizeof(packetHeader), 0, (sockaddr *)&remote, sizeofScoketAddrIn);
      } else if (buf.size() < windowSize){
        out.seekp(packet.header.seqnum * PACKETDATASIZE);
        out.write(packet.data, packet.header.length);
        buf.push(packet.header.seqnum);
      }
    }
  }
  stopReciver = 1;
  return ret;
}

void RdtReciver::timer() {
  clock_t startTime = clock();
  double secondPassed;
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
        sendto(socket, (char *)&packet, sizeof(packetHeader), 0, (sockaddr *)&remote, sizeofScoketAddrIn);
      }
      shouldRestartTimer = 1;
      startTime = clock();
    } else if (shouldRestartTimer) {
      cout << "restart time\n";
      startTime = clock();
      shouldRestartTimer = 0;
    }
  }
  cout << "timer end\n";
}