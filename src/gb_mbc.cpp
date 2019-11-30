#include "gb_mbc.h"
using namespace std;

MBC::MBC(unsigned char *p_ram, unsigned char *p_rom) {
  ram = p_ram;
  rom = p_rom;

  // must init curr_rom_bank to 1 for NoMBC
  // Otherwise doesn't matter
  curr_rom_bank = 1;
  curr_ram_bank = 0;
}

unsigned char MBC::read_byte(unsigned short address) {
  unsigned offset;

  if (address >= 0x0000 && address < 0x4000) {
    // ROM bank 0
    return rom[address];
  } else if (address >= 0x4000 && address < 0x8000) {
    // ROM bank 01-7F
    offset = (curr_rom_bank * 0x4000) + (address - 0x4000);
    return rom[offset];
  } else if (address >= 0xA000 && address < 0xC000) {
    if (ram_enable) {
      offset = (curr_ram_bank * Ram_Bank_Size) + (address - 0xA000);
      return ram[offset];
    } else {
      return 0xFF;
    }
  } else {
    cerr << hex << setfill('0') << setw(4) << unsigned(address) << " ";
    cerr << "unimplemented memory range in gb_mbc.cpp:write_byte" << endl;
    // cpu.stop = true;
    return 0xFF;
  }
}

void NoMBC::write_byte(unsigned short address, unsigned char value) { ; }

void MBC1::write_byte(unsigned short address, unsigned char value) {
  unsigned char tmp;
  unsigned offset;

  if (address >= 0x0000 && address < 0x2000) {
    // Enable/Disable external RAM
    if ((value & 0x0F) == 0x0A)
      ram_enable = true;
    else
      ram_enable = false;
  } else if (address >= 0x2000 && address < 0x4000) {
    // ROM bank number (lower 5 bits)
    tmp = value & 0x1F; // only care about lower 5 bits
    if ((tmp == 0x00) || (tmp == 0x20) || (tmp == 0x40) || (tmp == 0x60)) {
      tmp++;
    }
    curr_rom_bank = (curr_rom_bank & 0x60) | (tmp & 0x1F);
  } else if (address >= 0x4000 && address < 0x6000) {
    // RAM bank number or ROM bank number (upper 2 bits). depends on mode
    if (mbc_mode == RAM_BANK) {
      // RAM bank number in range 0-3
      curr_ram_bank = (value & 0x03);
    } else if (mbc_mode == ROM_BANK) {
      // Upper 2 bits of ROM bank number
      curr_rom_bank = ((value << 5) & 0x60) | (curr_rom_bank & 0x1F);
    }
  } else if (address >= 0x6000 && address < 0x8000) {
    // ROM/RAM mode Select
    if (value & 0x01) {
      mbc_mode = RAM_BANK;
    } else {
      mbc_mode = ROM_BANK;
    }
  } else if (address >= 0xA000 && address < 0xC000) {
    // RAM bank
    if (ram_enable) {
      offset = (curr_ram_bank * Ram_Bank_Size) + (address - 0xA000);
      ram[offset] = value;
    }
  } else {
    // cpu.stop = true;
    cerr << hex << setfill('0') << setw(4) << unsigned(address) << " ";
    cerr << "unimplemented memory range in gb_mbc.cpp:mbc1:write_byte" << endl;
  }
}

void MBC3::write_byte(unsigned short address, unsigned char value) {
  unsigned char tmp;
  unsigned offset;

  if (address >= 0x0000 && address < 0x2000) {
    // Enable/Disable external RAM
    if (value == 0x0A)
      ram_enable = true;
    else if (value == 0x00)
      ram_enable = false;
  } else if (address >= 0x2000 && address < 0x4000) {
    // ROM bank number (lower 5 bits)
    tmp = value & 0x7F;
    if (tmp == 0x00) {
      tmp++;
    }
    curr_rom_bank = tmp;
  } else if (address >= 0x4000 && address < 0x6000) {
    // RAM bank number or ROM bank number (upper 2 bits). depends on mode
    if (value >= 0x00 && value <= 0x07) {
      curr_ram_bank = value;
    } else if (value >= 0x08 && value <= 0x0C) {
      ; // TODO RTC
    }
  } else if (address >= 0x6000 && address < 0x8000) {
    // ROM/RAM mode Select
    if (value & 0x01) {
      mbc_mode = RAM_BANK;
    } else {
      mbc_mode = ROM_BANK;
    }
  } else if (address >= 0xA000 && address < 0xC000) {
    // RAM bank
    if (ram_enable) {
      offset = (curr_ram_bank * Ram_Bank_Size) + (address - 0xA000);
      ram[offset] = value;
    }
  } else {
    // cpu.stop = true;
    cerr << hex << setfill('0') << setw(4) << unsigned(address) << " ";
    cerr << "unimplemented memory range in gb_cart.cpp:mbc3:write_byte" << endl;
  }
}

void MBC5::write_byte(unsigned short address, unsigned char value) {
  unsigned char tmp;
  unsigned offset;

  if (address >= 0x0000 && address < 0x2000) {
    // Enable/Disable external RAM
    if (value == 0x0A)
      ram_enable = true;
    else if (value == 0x00)
      ram_enable = false;
  } else if (address >= 0x2000 && address < 0x3000) {
    // ROM bank number (lower 5 bits)
    curr_rom_bank = (curr_rom_bank & 0x0100) | (value & 0x00FF);
  } else if (address >= 0x3000 && address < 0x4000) {
    // ROM bank number (lower 5 bits)
    tmp = (value & 0x01);
    curr_rom_bank = (tmp & 0x0100) | (curr_rom_bank & 0x00FF);
  } else if (address >= 0x4000 && address < 0x6000) {
    // RAM bank number or ROM bank number (upper 2 bits). depends on mode
    if (value >= 0x00 && value <= 0x0F) {
      curr_ram_bank = value;
    } else {
      ;
      cerr << "mbc5:write_byte. undefined value for rom bank num" << endl;
    }
  } else if (address >= 0x6000 && address < 0x8000) {
    ;
  } else if (address >= 0xA000 && address < 0xC000) {
    // RAM bank
    if (ram_enable) {
      offset = (curr_ram_bank * Ram_Bank_Size) + (address - 0xA000);
      ram[offset] = value;
    }
  } else {
    // cpu.stop = true;
    cerr << hex << setfill('0') << setw(4) << unsigned(address) << " ";
    cerr << "unimplemented memory range in gb_cart.cpp:mbc3:write_byte" << endl;
  }
}
