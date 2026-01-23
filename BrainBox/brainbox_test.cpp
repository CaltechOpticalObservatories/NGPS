
#include "brainbox.h"

int bye() {
  std::cout << "exiting!\n";
  return 1;
}

std::optional<BrainBox::Interface> controller;

enum class Signal { Interlock,
                    LampControl,
                    LampStatus,
                    LaserStatus,
                    LampFault,
                    ControllerFault
                  };

static constexpr std::array<std::pair<std::string_view, Signal>, 6>
  signal_table{ {
    { "interlock",       Signal::Interlock },
    { "lampcontrol",     Signal::LampControl },
    { "lampstatus",      Signal::LampStatus },
    { "laserstatus",     Signal::LaserStatus },
    { "lampfault",       Signal::LampFault },
    { "controllerfault", Signal::ControllerFault }
  } };

std::optional<Signal> signal_from_string(std::string_view namein) {
  for (auto &[name, sig] : signal_table) {
    if (name==namein) return sig;
  }
  return std::nullopt;
}

std::vector<std::string_view> list_signals() {
  std::vector<std::string_view> names;
  names.reserve(signal_table.size());
  for (auto &[name, _] : signal_table) {
    names.push_back(name);
  }
  return names;
}

int configure_dio(const std::vector<std::string> &input) {

   if (!controller) {
     std::cout << "ERROR no lamp controller has been configured" << std::endl;
     return 1;
   }

   std::vector<BrainBox::Interface::PinConfig> pins;

   // each vector element is a line from the config file
   for (const auto &line : input) {
     std::istringstream iss(line);

     int pinin;
     std::string dirin, namein;

     if (!(iss >> namein >> pinin >> dirin)) {
       std::cout << "ERROR exepected <name> <pin#> <direction>" << std::endl;
       return 1;
     }

     if (pinin < 0 || pinin > 7) {
       std::cout << "ERROR invalid pin '" << pinin << "' must be in range {0:7}" << std::endl;
       return 1;
     }

     make_uppercase(dirin);

     if (dirin != "INPUT" && dirin != "OUTPUT") {
       std::cout << "ERROR invalid direction '" << dirin << "' must be input|output" << std::endl;
       return 1;
     }

     try {
       auto dir = BrainBox::direction_from_string(dirin);
       if (!dir) throw std::runtime_error("unknown direction: '"+dirin+"'");

       auto signal = signal_from_string(namein);
       if (!signal) throw std::runtime_error("unknown signal: '"+namein);

       pins.push_back( { pinin, *dir } );
     }
     catch (const std::exception &e) {
       std::cout << "ERROR parsing <pin#> <direction> <name> : " << e.what() << std::endl;
       return 1;
     }
   }

   return controller->configure_pins(pins);
 }

int main(int argc, char** argv) {
  const std::string host="10.0.0.60";
  const int port=9500;
  const int address=1;

  controller.emplace(host, port, address);

  std::vector<std::string> dioconfig {"interlock 0 output ",
                                      "lampcontrol 1 output ",
                                      "lampstatus 4 input  ",
                                      "laserstatus 5 input  ",
                                      "lampfault 6 input  ",
                                      "controllerfault 7 input  "};

  configure_dio(dioconfig);

  bool isinit = controller->get_initialized();
  std::cout << "BrainBox is" << (isinit ? " " : " not ") << "initialized\n";
  if (!isinit) return bye();


  auto signals = list_signals();
  std::cout << "signal list:" << std::endl;
  for (const auto &sig : signals) std::cout << "  " << sig << std::endl;

  if ( controller->open() != NO_ERROR ) return bye();

  std::string retstring;

  controller->send_command("$01M", retstring);
  std::cout << "device name = " << retstring << std::endl;

  controller->close();

  /***
  controller.configure_dio(dioconfig);

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
