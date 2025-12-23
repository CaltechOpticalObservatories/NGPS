
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

  std::vector<std::string> dioconfig {"interlock 0 output ",
                                      "lampcontrol 1 output ",
                                      "lampstatus 4 input  ",
                                      "laserstatus 5 input  ",
                                      "lampfault 6 input  ",
                                      "controllerfault 7 input  "};

  controller.configure_dio(dioconfig);
  /***

  bool isinit = controller.get_initialized();

  std::cout << "BrainBox is" << (isinit ? " " : " not ") << "initialized\n";

  if (!isinit) return bye();

  if ( controller.open() != NO_ERROR ) return bye();

  std::string retstring;

  controller.send_command("$01M", retstring);
  std::cout << "device name = " << retstring << std::endl;

  controller.send_command("$01M0", retstring);
  std::cout << "device model = " << retstring << std::endl;

  controller.send_command("$01M1", retstring);
  std::cout << "device location = " << retstring << std::endl;

  controller.send_command("$012", retstring);
  std::cout << "device status = " << retstring << std::endl;

  controller.send_command("$016", retstring);
  std::cout << "DIO channel values = " << retstring << std::endl;

  controller.send_command("@01", retstring);
  std::cout << "DIO status = " << retstring << std::endl;

  controller.send_command("$01L0", retstring);
  std::cout << "latched digital inputs low = " << retstring << std::endl;

  controller.send_command("$01L1", retstring);
  std::cout << "latched digital inputs high = " << retstring << std::endl;

  controller.read_digio();

  for (int i=0; i<=7; i++) {
    auto state = controller.get_diostate(i);
    if (state.has_value()) {
      std::cout << "pin state " << i << " " << (*state ? "set" : "clear") << "\n";
    }
  }

  std::cout << "\nDIO pin names:\n";
  auto names = controller.get_dionames();
  for (const auto &name : names) std::cout << "  " << name << std::endl;

  {
  auto state = controller.get_diostate("lampstatus");
  std::cout << "\nDIO state lampstatus = " << (*state ? "set" : "clear") << "\n";
  }

  bool is_interlock;
  controller.read_digio();
  {
  auto state = controller.get_diostate("interlock");
  is_interlock = *state;
  std::cout << "\nDIO state interlock = " << (is_interlock ? "set" : "clear") << "\n";
  }

  try {
    controller.set_bit("interlock", (is_interlock?false:true));
  }
  catch (const std::exception &e) {
    std::cout << "ERROR " << e.what() << std::endl;
    controller.close();
    exit(1);
  }

  controller.read_digio();
  {
  auto state = controller.get_diostate("interlock");
  is_interlock = *state;
  std::cout << "\nDIO state interlock = " << (is_interlock ? "set" : "clear") << "\n";
  }

  controller.close();
  ***/

  return 0;
}
