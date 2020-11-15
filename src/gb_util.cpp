#include "gb_util.h"
#include <iostream>
#include <sstream>
#include <limits>

namespace gb_util {
std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(s);
  while (std::getline(tokenStream, token, delim)) {
    tokens.push_back(token);
  }
  return tokens;
}

std::string get_console_line(void) {
  std::cin.clear();
  std::string s;
  getline(std::cin, s);
  return s;
}

void put_console_line(const std::string s) { std::cout << s; }

unsigned short stous(std::string const &str, size_t *idx, int base) {
  unsigned long result = std::stoul(str, idx, base);
  if (result > std::numeric_limits<unsigned short>::max()) {
    throw std::out_of_range("stous");
  }
  return static_cast<unsigned short>(result);
}
} // namespace gb_util
