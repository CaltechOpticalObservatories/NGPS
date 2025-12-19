
#include "brainbox.h"

int bye() {
  std::cout << "exiting!\n";
  return 1;
}

int main(int argc, char** argv) {
  const std::string host="10.0.0.60";
  const int port=9500;
  const int address=1;

  BrainBox::Interface controller(host, port, address);

  bool isinit = controller.get_initialized();

  std::cout << "BrainBox is" << (isinit ? " " : " not ") << "initialized\n";

  if (!isinit) return bye();

  if ( controller.open() != NO_ERROR ) return bye();

  controller.close();

  return 0;
}
