#include "gb_memory.h"
#include "gb_lcd.h"
#include "gb_int.h"
#include "gb_timer.h"
#include "gb_cart.h"
#include "gb_cpu.h"

bool remapped_cart = false;

void Memory::init_bios(string path) {
  ifstream bios_file;
  int file_len;

  bios_file.open(path, ios::in | ios::binary);
  if (bios_file) {
    // get length of file
    bios_file.seekg(0, bios_file.end);
    file_len = bios_file.tellg();
    bios_file.seekg(0, bios_file.beg);

    if (file_len != BOOT_ROM_SIZE) {
      std::cerr << "GBcon: Error: bad len for gb bios" << std::endl;
      // should probably throw an exception here
      return;
    }

    bios_file.read((char *)boot_rom, file_len);
  } else {
    std::cerr << "GBcon: Error: could not open bios file" << std::endl;
    return;
  }
}

void Memory::oam_dma_transfer(unsigned short dst, unsigned short src, size_t length) {
  for (unsigned int i = 0; i < length; i++)
    write_byte(dst + i, read_byte(src + i));
}

unsigned char Memory::read_byte(unsigned short address) {

  if (address >= 0x000 && address < 0x8000) {
    if (address <= 0x0100 && remapped_cart == false) {
      return boot_rom[address];
    } else {
      return cart->read_byte(address);
    }
  } else if (address >= 0x8000 && address < 0xA000) {
    return vram[address - 0x8000];
  } else if (address >= 0xA000 && address < 0xC000) {
    return cart->read_byte(address);
  } else if (address >= 0xC000 && address < 0xE000) {
    return sram[address - 0xC000];
  } else if (address >= 0xE000 && address < 0xFE00) {
    // FIXME echo ram. i should probably do something about mirroring but eh
    return eram[address - 0xE000];
  } else if (address >= 0xFE00 && address < 0xFEA0) {
    return oram[address - 0xFE00];
  } else if (address >= 0xFEA0 && address < 0xFF00) {
    return unused[address - 0xFEA0];
  } else if (address == 0xFF00) {
    if ((ioram[address - 0xFF00] & 0x20) == 0) { 
      // PIO15 is low. Return buttons
      ioram[address - 0xFF00] = (ioram[address - 0xFF00] & 0xF0) |
                                (~buttons & 0x0F);
    } else if ((ioram[address - 0xFF00] & 0x10) == 0) { 
      // PIO14 is low. Return directions
      ioram[address - 0xFF00] = (ioram[address - 0xFF00] & 0xF0) |
                                (~direction & 0x0F);
    } else { // undefined
      std::cerr << "GBcon: Error. tried to read ff00 with bad input" << std::endl;
    }
    return ioram[address - 0xFF00];
  } else if (address == 0xFF01) {
    return ioram[address - 0xFF00];
  } else if (address == 0xFF02) {
    return 0x00; // SIO isn't fully implemented
  } else if (address == 0xFF04) {
    return timer->div;
  } else if (address == 0xFF05) {
    return timer->tima;
  } else if (address == 0xFF06) {
    return timer->tma;
  } else if (address == 0xFF07) {
    return timer->get_tac();
  } else if (address == 0xFF0F) {
    return (interrupt->flags & 0x1F);
  } else if (address >= 0xFF10 && address <= 0xFF3F) {
    return 0x00;
    //  return sound->regRead(address);
  } else if (address == 0xFF40) {
    return lcd->get_lcdc();
  } else if (address == 0xFF41) {
    return lcd->get_stat();
  } else if (address == 0xFF42) {
    return lcd->get_scy();
  } else if (address == 0xFF43) {
    return lcd->get_scx();
  } else if (address == 0xFF44) {
    return lcd->get_ly();
  } else if (address == 0xFF45) {
    return lcd->get_lyc();
    // 0xFF46 is write only
  } else if (address == 0xFF47) {
    return lcd->get_bgp();
  } else if (address == 0xFF48) {
    return lcd->get_obp0();
  } else if (address == 0xFF49) {
    return lcd->get_obp1();
  } else if (address == 0xFF4A) {
    return lcd->get_wy();
  } else if (address == 0xFF4B) {
    return lcd->get_wx();
  } else if (address >= 0xFF00 && address < 0xFF80) {
    return ioram[address - 0xFF00];
  } else if (address >= 0xFF80 && address < 0xFFFF) {
    return hram[address - 0xFF80];
  } else if (address == 0xFFFF) { // 0xFFFF
    return interrupt->en;
  } else {
    std::cerr << "GBcon: Error: reading from invalid memory address " << hex
         << unsigned(address) << std::endl;
    cpu->stop = true;
  }

  return 0;
}

unsigned short Memory::read_short(unsigned short address) {
  return ((read_byte(address) << 0) & 0x00FF) |
         ((read_byte(address + 1) << 8) & 0xFF00);
}

unsigned short Memory::read_short_stack(unsigned short sp) {
  return read_short(sp);
}

void Memory::write_byte(unsigned short address, unsigned char value) {

  if (address >= 0x000 && address < 0x8000) {
    if (address <= 0x0100 && remapped_cart == false) {
      boot_rom[address] = value;
    } else {
      cart->write_byte(address, value);
    }
    // cart[address] = value;
  } else if (address >= 0x8000 && address < 0xA000) {
    vram[address - 0x8000] = value;
  } else if (address >= 0xA000 && address < 0xC000) {
    cart->write_byte(address, value);
    // cram[address - 0xA000] = value;
  } else if (address >= 0xC000 && address < 0xE000) {
    sram[address - 0xC000] = value;
  } else if (address >= 0xE000 && address < 0xFE00) {
    // TODO echo ram so i should probably do something about mirroring but eh
    eram[address - 0xE000] = value;
  } else if (address >= 0xFE00 && address < 0xFEA0) {
    oram[address - 0xFE00] = value;
  } else if (address >= 0xFEA0 && address < 0xFF00) {
    unused[address - 0xFEA0] = value;
  } else if (address == 0xFF04) {
    timer->div = 0;
  } else if (address == 0xFF05) {
    timer->tima = value;
  } else if (address == 0xFF06) {
    timer->tma = value;
  } else if (address == 0xFF07) {
    timer->set_tac(value);
  } else if (address == 0xFF0F) {
    interrupt->flags = (value & 0x1F);
  } else if (address >= 0xFF10 && address <= 0xFF3F) {
    //  sound->regWrite(address, value);
  } else if (address == 0xFF40) {
    lcd->set_lcdc(value);
  } else if (address == 0xFF41) {
    lcd->set_stat(value);
  } else if (address == 0xFF42) {
    lcd->set_scy(value);
  } else if (address == 0xFF43) {
    lcd->set_scx(value);
  } else if (address == 0xFF45) {
    lcd->set_lyc(value);
  } else if (address == 0xFF46) {
    oam_dma_transfer(0xFE00, value << 8, 160);
  } else if (address == 0xFF47) {
    lcd->set_bgp(value);
  } else if (address == 0xFF48) {
    lcd->set_obp0(value);
  } else if (address == 0xFF49) {
    lcd->set_obp1(value);
  } else if (address == 0xFF4A) {
    lcd->set_wy(value);
  } else if (address == 0xFF4B) {
    lcd->set_wx(value);
  } else if (address == 0xFF00) { // joypad
    ioram[address - 0xFF00] = (value & 0xF0);
  } else if (address == 0xFF02) { // SIO control
    // hacky SIO implementation
    if (value == 0x81) {
      serial_tx_data = ioram[0x4401 - 0x4400];
      serial_tx_initd = true;
    }
  } else if (address >= 0xFF00 && address < 0xFF80) {
    ioram[address - 0xFF00] = value;
  } else if (address >= 0xFF80 && address < 0xFFFF) {
    hram[address - 0xFF80] = value;
  } else if (address == 0xFFFF) { // 0xFFFF
    interrupt->en = value;
  } else {
    std::cerr << "GBcon: Error: writing to invalid memory address " << hex
         << unsigned(address) << std::endl;
    cpu->stop = true;
  }
}

void Memory::write_short(unsigned short address, unsigned short value) {
  write_byte(address, ((value >> 0) & 0x00FF));
  write_byte(address + 1, ((value >> 8) & 0x00FF));
}

void Memory::write_short_to_stack(unsigned short sp, unsigned short value) {
  write_byte(sp - 1, ((value >> 8) & 0x00FF));
  write_byte(sp - 2, ((value >> 0) & 0x00FF));
}

void Memory::print_memory_range(unsigned short start_addr,
                                unsigned short blocks) {
  unsigned short end_addr, addr;
  start_addr &= 0xFFF0;
  end_addr = start_addr + (0x000F * blocks);

  for (unsigned short i = (start_addr >> 4); i <= (end_addr >> 4); i++) {
    std::cout << hex << setfill('0') << setw(4) << unsigned(i << 4) << " | ";
    for (unsigned short j = 0; j <= 0xF; j++) {
      addr = (i << 4) + j;
      std::cout << hex << setfill('0') << setw(2) << unsigned(read_byte(addr))
           << " ";
    }
    std::cout << std::endl;
  }
}

void Memory::print_to_file(std::string filepath) {
  std::ofstream ofs(filepath, std::ofstream::out | std::ofstream::trunc);
  unsigned short addr;

  for (unsigned short i = 0; i <= 0x0FFF; i++) {
    ofs << hex << setfill('0') << setw(4) << unsigned(i << 4) << " | ";
    for (unsigned short j = 0; j <= 0xF; j++) {
      addr = (i << 4) + j;
      ofs << hex << setfill('0') << setw(2) << unsigned(read_byte(addr)) << " ";
    }
    ofs << std::endl;
  }
  ofs.close();
}

void Memory::init(GB_Sys *gb_sys) {
  cpu = gb_sys->cpu;
  lcd = gb_sys->lcd;
  interrupt = gb_sys->interrupt;
  cart = gb_sys->cart;
  timer = gb_sys->timer;
}
