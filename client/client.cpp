#include <winsock2.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include "packet.hpp"
#include "sender.hpp"
// #include "Rdt.hpp"
using namespace std;

int main(int argc, char const *argv[]) {
  RdtSender sender;
  // Rdt rdt;
  if (sender.init(inet_addr("127.0.0.1"), 8888)) {
    cout << "Rdt init failed\n";
    return 0;
  }

  sender.rdt_send_file("./data/1.mp3");
  
  return 0;
}
