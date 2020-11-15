#include "gb_cpu.h"
#include "gbcon.h"
#include <vector>
#include <string>
#include <functional>
#include <map>

class Debug {
public:
  void run(void);
  void init(GB_Sys *gb_sys);

  void write_serial_log_file(std::string filepath);
  bool stopped;

private:
  // pointers to other components
  CPU *cpu;
  Memory *mem;

  struct Watchpoint {
    unsigned short addr;
    unsigned char prev_val;
    bool enabled;
  };
  struct Breakpoint {
//  unsigned short addr;
    bool enabled;
  };

  bool check_watchpoints(void);
  bool check_breakpoints(void);

  std::map<unsigned short, Breakpoint> breakpoints;
  std::map<unsigned short, Watchpoint> watchpoints;

  // Debug Command Handlers

  void exec_dbg_command(std::vector<std::string> &argv);

  typedef std::vector<std::string> CmdArgs;
  void execute_help_cmd(CmdArgs &argv);
  void execute_break_cmd(CmdArgs &argv);
  void execute_watch_cmd(CmdArgs &argv);
  void execute_continue_cmd(CmdArgs &argv);
  void execute_step_cmd(CmdArgs &argv);
  void execute_examine_cmd(CmdArgs &argv);
  void execute_info_cmd(CmdArgs &argv);
  void execute_reset_cmd(CmdArgs &argv);
  void execute_quit_cmd(CmdArgs &argv);

  struct CommandDef {
    const std::string name;
    std::function<void (CmdArgs&)> handler;
    const std::string desc;
  };

  std::vector<CommandDef> commands;

  void print_cpu_state(void);
  void print_prompt(void);
  void add_command(std::string name, void (Debug::*func)(CmdArgs&), std::string desc);
  bool step_resume = false;
  // width used to print help commands
  const int PrintWidth = 16;

  std::vector<char> serial_data;
};
