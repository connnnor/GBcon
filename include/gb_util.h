#include <vector>
#include <string>

namespace gb_util {
  std::vector<std::string> split(const std::string& s, char delim);
  std::string get_console_line(void);

  unsigned short stous(std::string const &str, size_t *idx = 0, int base = 10);
}
