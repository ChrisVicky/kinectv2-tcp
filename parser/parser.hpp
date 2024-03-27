#ifndef PARSER_HPP
#define PARSER_HPP
#include "json.hpp"
using namespace nlohmann;
bool parseAddressIp(std::string &js, std::string &address, int &ip);
#endif
