#include "parser.hpp"
#include <iostream>
bool parseAddressIp(std::string &js, std::string &address, int &port) {
  /* std::cout << "Js: " << js << std::endl; */
  json data = json::parse(js);
  address = data["address"];
  port = data["port"];
  return true;
}
