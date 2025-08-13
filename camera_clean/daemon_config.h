#pragma once
#include <string>

namespace Camera::Daemon {
  inline const std::string NAME = "camerad";
  inline const std::string STDOUT = "/dev/null";
  inline const std::string STDERR = "/tmp/"+NAME+".stderr";
};
