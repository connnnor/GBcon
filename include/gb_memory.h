#pragma once
#include "gbcon.h"
#include <fstream>
#include <iomanip>
#include <iostream>

#define BOOT_ROM_SIZE 0x100

extern unsigned char buttons;
extern unsigned char direction;
using namespace std;

class Memory {
public:
  unsigned short last_mem_addr_dbg;
  unsigned char read_byte(unsigned short address);
  unsigned short read_short(unsigned short address);
  unsigned short read_short_stack(unsigned short sp);

  void write_byte(unsigned short address, unsigned char value);
  void write_short(unsigned short address, unsigned short value);
  void write_short_to_stack(unsigned short sp, unsigned short value);

  void oam_dma_transfer(unsigned short dst, unsigned short src, size_t length);
  void init_bios(string bios_path);
  /* debug and helpers */
  void print_memory_range(unsigned short start_addr, unsigned short blocks);
  void print_to_file(string log_dir);

  unsigned char boot_rom[BOOT_ROM_SIZE];

  unsigned char serial_tx_data;
  unsigned char serial_rx_data;
  bool serial_tx_initd = false;
  bool remapped_cart = false;

  void init(GB_Sys *gb_sys);

//private:
  /* GB system (pointers to other components)
   */

  CPU       *cpu;
  Cartridge *cart;
  LCD       *lcd;
  Interrupt *interrupt;
  Timer     *timer;

  /* Gameboy Memory Map
  Start	End	Description	Notes
  0000	3FFF	16KB ROM bank 00	From cartridge,
  4000	7FFF	16KB ROM Bank 01~NN	From cartridge, switchable
  8000	9FFF	8KB Video RAM (VRAM)	Only bank 0 in Non-CGB mode
  Switchable bank 0/1 in CGB mode

  A000	BFFF	8KB External RAM	In cartridge, switchable
  C000	CFFF	4KB Work RAM (WRAM) bank 0
  D000	DFFF	4KB Work RAM (WRAM) bank 1~N	Only bank 1 in Non-CGB mode
  Switchable bank 1~7 in CGB mode

  E000	FDFF	Mirror of C000~DDFF (ECHO RAM)	Typically not used
  FE00	FE9F	Sprite attribute table (OAM)
  FEA0	FEFF	Not Usable
  FF00	FF7F	I/O Registers
  FF80	FFFE	High RAM (HRAM)
  FFFF	FFFF	Interrupts Enable Register (IE)
  */

  // 0x0000-0x7FFF. From Cartridge
  // use cart->read_byte(address)
  // 0x8000-0x9FFF. Vide RAM
  unsigned char vram[0x2000];
  const unsigned short Vram_Addr = 0x8000;
  const unsigned short Vram_Size = 0x2000;
  // 0xA000-0xBFFF. External RAM inside cartridge
  // use cart->read_byte(address)
  // 0xC000-0xDFFF. Work RAM inside Gameboy
  unsigned char sram[0x2000];
  const unsigned short Sram_Addr = 0xC000;
  const unsigned short Sram_Size = 0x2000;
  // 0xE000-0xFDFF. echo ram (not used)
  unsigned char eram[0x1E00];
  const unsigned short Eram_Addr = 0xE000;
  const unsigned short Eram_Size = 0x1E00;
  // 0xFE00-0xFE9F. Object Attribute Memory
  unsigned char oram[0x00A0];
  const unsigned short Oram_Addr = 0xFE00;
  const unsigned short Oram_Size = 0x00A0;
  // 0xFEA0-0xFEFF. Unusable
  unsigned char unused[0x0060];
  const unsigned short Unused_Addr = 0xFEA0;
  const unsigned short Unused_Size = 0x0060;
  // 0xFF00-0xFF7F. I/O Registers
  unsigned char ioram[0x0080];
  const unsigned short Ioram_Addr = 0xFF00;
  const unsigned short Ioram_Size = 0x0080;
  // 0xFF80-0xFFFE
  unsigned char hram[0x007F];
  const unsigned short Hram_Addr = 0xFF80;
  const unsigned short Hram_Size = 0x007F;
  // 0xFFFF. Interrupt Enable
  // interrupt.en = value;
};
