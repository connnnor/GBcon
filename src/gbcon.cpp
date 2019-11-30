#include "gb_cpu.h"
#include "gb_dbg.h"
#include "gb_lcd.h"
#include "gb_sdl.h"
#include "gb_cart.h"
#include "gb_timer.h"
#include "gb_int.h"
#include "gb_memory.h"
#include <fstream>
#include <iostream>
#include <boost/program_options.hpp>
#include <string>
#include <sys/time.h>
#include <unistd.h>

namespace po = boost::program_options;
// Interrupt interrupt;
bool log_files = false;
bool cpu_pause = false;

extern Sdl_emu_keys emu_keys;

CPU cpu;
Memory mem;
LCD lcd;
Interrupt interrupt;
Timer timer;
Debug dbg;
Cartridge *cart;

string bios_path, rom_path, log_dir, dbg_flag, sav_path;

void handle_emu_input(void) {
  /* save ram & load ram */
  if (emu_keys.save_ram) {
    cart->export_sav(sav_path);
    emu_keys.save_ram = false;
  }
}

int main(int argc, char *argv[]) {
  int scale_factor;

  /** Parse command line arguements
   */

  try {
    po::options_description desc("Allowed options");
    // first param           = option name & short name
    // optional middle param = parameter to option
    // last param            = description
    desc.add_options()
      ( "help", "Display help message")
      ( "rom,r", po::value<string>(&rom_path)->required(), "path to rom")
      ( "bios,b", po::value<string>(&bios_path), "path to bios")
      ("log,l", po::value<string>(&log_dir), "log directory")
      ( "dbg,d", po::bool_switch(&dbg.stopped)->default_value(false), 
        "start emu in debugger")
      ("scale,s", po::value<int>(&scale_factor)->default_value(1),
       "display scale. 1, 2, 4");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
      if (vm.count("help")) {
        std::cout << desc << std::endl;
        return EXIT_SUCCESS;
      }

      // throws if required arg isn't specified
      po::notify(vm);
  } catch(po::error& e) {
      std::cerr << e.what() << std::endl;
      return EXIT_FAILURE;
  } catch(...) {
      std::cerr << "GBcon: Unknown error while parsing args" << std::endl;
      return EXIT_FAILURE;
  }

  sav_path = rom_path + ".sav";

  /** GBcon code
   */

  // read in bios and rom
  cart = new Cartridge(rom_path);
  if (!cart->loaded) {
    return EXIT_FAILURE;
  }

  if (!bios_path.empty()) {
    mem.init_bios(bios_path);
  }

  // initialize sdl after checking params
  Sdl_params sdl_p;
  switch (scale_factor) {
  case 1:
  case 2:
  case 4:
    // supported scale factors
    sdl_p.scale = scale_factor;
    break;
  default:
    std::cerr << "GBcon: unsupported scale factor " << scale_factor << std::endl;
    sdl_p.scale = 1;
    break;
  }
  sdl_init(sdl_p);

  // pass around pointers
  GB_Sys gb_sys;
  gb_sys.cpu = &cpu;
  gb_sys.lcd = &lcd;
  gb_sys.mem = &mem;
  gb_sys.interrupt = &interrupt;
  gb_sys.cart = cart;
  gb_sys.timer = &timer;
  gb_sys.dbg = &dbg;

  cpu.init(&gb_sys);
  lcd.init(&gb_sys);
  interrupt.init(&gb_sys);
  mem.init(&gb_sys);
  timer.init(&gb_sys);
  dbg.init(&gb_sys);

  // reset cpu, then loop
  cpu.reset();

  bool user_quit = false;
  unsigned int clksLeft; // clock cycles -> 4.19Mhz
  while (cpu.stop == false && user_quit == false) {
    // drop into debugger
    dbg.run();

    // save ram if pressed
    handle_emu_input(); //FIXME - move out of main loop

    // exec instruction and get num cycles taken
    clksLeft = cpu.cpu_step();

    // step other subsystems (just LCD for now)
    while (clksLeft) {
      user_quit |= lcd.step(4);
      clksLeft -= 4;
    }

    // check interrupts
    interrupt.step();

    // remap bootrom area to cartridge ram after 0x0100
    if (cpu.registers.pc == 0x0100) {
      mem.remapped_cart = true;
    }
  }

  if (!log_dir.empty()) {
    std::cout << "GBcon: writing logs to " << log_dir << std::endl;
    mem.print_to_file(log_dir + "/memdump.log");
    dbg.write_serial_log_file(log_dir + "/serial.log");
  }

  sdl_uninit();
  return 0;
}

