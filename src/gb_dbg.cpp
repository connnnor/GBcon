#include "gb_dbg.h"
#include "gb_memory.h"
#include "gb_util.h"
#include <iostream>

void Debug::run(void) {
  // check events
  stopped |= check_breakpoints();
  stopped |= check_watchpoints();
  stopped |= step_resume;

  step_resume = false;

  // drop into debugger prompt for break/watch/step
  while (stopped) {
    print_prompt();
    // read command line input
    std::string cmd_text = gb_util::get_console_line();
    // tokenize and execute
    std::vector<std::string> argv = gb_util::split(cmd_text, ' ');
    // execute command handler
    exec_dbg_command(argv);
  }

  // append new serial data to buffer
  if (mem->serial_tx_initd) {
    serial_data.push_back(mem->serial_tx_data);
    mem->serial_tx_initd = false;
  }
}

void Debug::init(GB_Sys *gb_sys) {
  cpu = gb_sys->cpu;
  mem = gb_sys->mem;

  // add commands
  add_command("help", &Debug::execute_help_cmd, "prints help message");
  add_command("break", &Debug::execute_break_cmd, "sets breakpoint at address");
//  add_command("watch", &Debug::execute_watch_cmd, "sets memory watchpoint at address");
  add_command("continue", &Debug::execute_continue_cmd, "continue emulator execution");
  add_command("step", &Debug::execute_step_cmd, "execute a single instruction");
  add_command("examine", &Debug::execute_examine_cmd, "display memory contents");
  //add_command("info", &Debug::execute_info_cmd, "prints name and values of registers");
  //add_command("reset", execute_reset_cmd,"reset emulator.");
  add_command("quit", &Debug::execute_quit_cmd, "quits emulator");
}

bool Debug::check_breakpoints(void) {
  if (breakpoints.count(cpu->registers.pc)) {
     if (breakpoints.at(cpu->registers.pc).enabled ) {
       print_cpu_state();
       return true;
     }
  } 
  return false;
}

bool Debug::check_watchpoints(void) {
  for (auto &w : watchpoints) {
    if (!w.second.enabled) {
      continue;
    }

    if (mem->read_byte(w.second.addr) != w.second.prev_val) {
      print_cpu_state();
      return true;
    }
  }

  return false;
}

void Debug::print_cpu_state(void) {
  auto opcode = mem->read_byte(cpu->registers.pc);

  // output : 0xADDR | Instruction Disassembly
  std::cout << "0x" << hex << setfill('0') << setw(4) 
    << cpu->registers.pc 
    << " | " 
    << cpu->instrs[opcode].disassembly << "\n";

  // output for each reg: name 0xBEEF
  std::cout << "Registers: " << "\n";
  std::cout 
    << "af 0x" << hex << setfill('0') << setw(4)
          << unsigned(cpu->registers.af) << "\n"
    << "bc 0x" << hex << setfill('0') << setw(4) 
          << unsigned(cpu->registers.bc) << "\n"
    << "de 0x" << hex << setfill('0') << setw(4)
          << unsigned(cpu->registers.de) << "\n"
    << "hl 0x" << hex << setfill('0') << setw(4) 
          << unsigned(cpu->registers.hl) << "\n"
    << "sp 0x" << hex << setfill('0') << setw(4)
          << unsigned(cpu->registers.sp) << "\n"
    << "pc 0x" << hex << setfill('0') << setw(4) 
          << unsigned(cpu->registers.pc) << "\n";
}

void Debug::print_prompt(void) {
  std::cout << "> ";
}

// TODO(connor): Add a typedef for func arguement
void Debug::add_command(std::string name,
                        void (Debug::*func)(CmdArgs &),
                        std::string desc) {
  CommandDef cmd = {name, std::bind(func, this, std::placeholders::_1), desc};
  commands.push_back(cmd);
}

void Debug::exec_dbg_command(std::vector<std::string> &argv) {
  // search commands
  for (auto &c : commands) {
    if (argv[0] == c.name || argv[0][0] == c.name[0]) {
      try {
        c.handler(argv);
      } catch (std::out_of_range) {
        std::cout << "Error: Arg out of range"
                  << "\n";
      } catch (std::invalid_argument) {
        std::cout << "Error: Invalid arguement"
                  << "\n";
      }
    }
  }
}


void Debug::execute_help_cmd(CmdArgs &argv) {
  std::cout << "Debugger commands:" << "\n";
  for (auto &c : commands) {
    std::cout << std::left << std::setfill(' ') 
      << std::setw(PrintWidth) << c.name 
      << std::setw(PrintWidth) << "--" 
      << std::setw(PrintWidth) << c.desc << "\n";
  }
}

void Debug::execute_break_cmd(std::vector<std::string> &argv) {
  // insert breakpoint if it doesn't already exist
  auto addr = gb_util::stous(argv[1], nullptr, 0);
  if (breakpoints.count(addr)) {
    std::cout << "breakpoint exists\n";
  } else {
    breakpoints.insert({addr, Breakpoint{.enabled = true}});
    cout << "breakpoint set\n";
  }
}

void Debug::execute_watch_cmd(std::vector<std::string> &argv) {
  // insert watchpoint if it doesn't already exist
  auto addr = gb_util::stous(argv[1], nullptr, 0);
  if (watchpoints.count(addr)) {
    std::cout << "watchpoint exists\n";
  } else {
    watchpoints.insert({addr, Watchpoint{.addr=addr, .prev_val=0x00, .enabled = true}});
    cout << "watchpoint set\n";
  }
}

void Debug::execute_continue_cmd(std::vector<std::string> &argv) {
  stopped = false;
}

void Debug::execute_step_cmd(std::vector<std::string> &argv) {
  print_cpu_state();
  stopped = false;
  step_resume = true;
}

void Debug::execute_examine_cmd(std::vector<std::string> &argv) {
  unsigned short addr = std::stoul(argv[1], nullptr, 0);
  unsigned short blocks;
  // check if num blocks arg specified
  if (argv.size() == 3) {
    blocks = std::stoul(argv[2], nullptr, 0);
  } else {
    blocks = 1;
  }
  mem->print_memory_range(addr, blocks);
}

// TODO(connor): info command with lcd, cpu, and mem modifiers
//void Debug::execute_info_cmd(std::vector<std::string> &argv) {}

// TODO
//void Debug::execute_reset_cmd(std::vector<std::string> &argv) {}

void Debug::execute_quit_cmd(std::vector<std::string> &argv) {
  std::cout << "exiting\n";
  cpu->stop = true;
  stopped = false;
}

void Debug::write_serial_log_file(std::string filepath) {
  std::ofstream out(filepath, std::ofstream::out | std::ofstream::trunc);
  for (const auto &c : serial_data ) out << char(c); 
  out << "\0";
}
