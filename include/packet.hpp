#ifndef _PACKET_HPP_
#define _PACKET_HPP_

#define PACKETDATASIZE 1024

struct packetHeader {
  unsigned long seqnum;
  char fin;
  unsigned short length;
  unsigned long long fileSize;
  unsigned long remaindSize;
  unsigned long sendTime;
};

struct Packet {
  packetHeader header;
  char data[PACKETDATASIZE];
};

#endif