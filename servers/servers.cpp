#include <winsock2.h>
#include <fstream>
#include <iostream>
#include "reciver.hpp"
#include "packet.hpp"

using namespace std;

int main(int argc, char const *argv[]) {
  RdtReciver reciver;
  if (reciver.init(INADDR_ANY, 8888)) {
    cout << "Rdt init failed\n";
    return 0;
  }

  reciver.rdt_rcv_file("./data/1.mp3");

  return 0;
}
