#include "gb_cart.h"
#include "gb_sdl.h"
#include "gb_mbc.h"


Cartridge::Cartridge(std::string rom_path) {
  std::ifstream rom_file;
  std::ifstream sav_file;
  int file_len;

  this->loaded = true;
  rom_file.open(rom_path, std::ios::in | std::ios::binary);
  if (rom_file) {
    // get length of file
    rom_file.seekg(0, rom_file.end);
    file_len = rom_file.tellg(); // FIXME - I guess tellg isn't ideal?
    rom_file.seekg(0, rom_file.beg);

    if (file_len < 0 || (unsigned int) file_len > Max_Rom_Size) {
      std::cerr << "GBcon: Invalid length for rom" << std::endl;
      loaded = false;
      return;
    } 

    cart_rom = new unsigned char[file_len];
    rom_file.read((char *)cart_rom, file_len);

    switch (cart_rom[Ram_size_addr]) {
    case 0x00: // ram size = 0. alloc anyways and fill with 0xFFs
      num_ram_banks = 0;
      cart_ram = new unsigned char[One_KB * 8];
      memset(cart_ram, 0xFF, One_KB * 8);
      break;
    case 0x01: // ram size = 2 Kbytes
      num_ram_banks = 1;
      cart_ram = new unsigned char[One_KB * 2];
      break;
    case 0x02: // ram size = 8 Kbytes
      num_ram_banks = 1;
      cart_ram = new unsigned char[One_KB * 8];
      break;
    case 0x03: // ram size = 32 Kbytes
      num_ram_banks = 4;
      cart_ram = new unsigned char[One_KB * 32];
      break;
    case 0x04: // ram size = 128 Kbytes
      num_ram_banks = 16;
      cart_ram = new unsigned char[One_KB * 128];
      break;
    case 0x05: // ram size = 64 Kbytes
      num_ram_banks = 8;
      cart_ram = new unsigned char[One_KB * 64];
      break;
    default:
      std::cerr << "GBcon: invalid ram size in cart header at 0x0147" << std::endl;
      loaded = false;
      return;
    }

    switch (cart_rom[Mbc_type_addr]) {
    case 0x00: // ROM ONLY
      mbc = new NoMBC(cart_ram, cart_rom);
      break;
    case 0x01: // MBC1
    case 0x02: // MBC1+RAM
    case 0x03: // MBC1+RAM+BATTERY
      mbc = new MBC1(cart_ram, cart_rom);
      break;
//  case 0x05: // MBC2
//  case 0x06: // MBC2+BATTERY
//  case 0x08: // ROM+RAM
//  case 0x09: // ROM+RAM+BATTERY
//  case 0x0B: // MMM01
//  case 0x0C: // MMM01+RAM
//  case 0x0D: // MMM01+RAM+BATTERY
    case 0x0F: // MBC3+TIMER+BATTERY
    case 0x10: // MBC3+TIMER+RAM+BATTERY
    case 0x11: // MBC3
    case 0x12: // MBC3+RAM
    case 0x13: // MBC3+RAM+BATTERY
      mbc = new MBC3(cart_ram, cart_rom);
      break;
    case 0x19: // MBC5
    case 0x1A: // MBC5+RAM
    case 0x1B: // MBC5+RAM+BATTERY
    case 0x1C: // MBC5+RUMBLE
    case 0x1D: // MBC5+RUMBLE+RAM
    case 0x1E: // MBC5+RUMBLE+RAM+BATTERY
      mbc = new MBC5(cart_ram, cart_rom);
      break;
//  case 0x20: // MBC6
//  case 0x22: // MBC7+SENSOR+RUMBLE+RAM+BATTERY
    default:
      std::cerr << "GBcon: Unsupported cartridge type" << std::endl;
      loaded = false;
      return;
    }

    // check if there is a .gb.sav file for this cart
    import_sav(rom_path + ".sav");
  } else {
    std::cerr << "GBcon: Error reading rom file" << std::endl;
    loaded = false;
  }
}

unsigned char Cartridge::read_byte(unsigned short address) {
  return mbc->read_byte(address);
}
void Cartridge::write_byte(unsigned short address, unsigned char value) {
  mbc->write_byte(address, value);
}

void Cartridge::import_sav(std::string path) {
  int file_len;
  std::ifstream sav_file;

  sav_file.open(path, std::ios::in | std::ios::binary);
  if (sav_file) {
    std::cout << "GBcon: sav file found" << std::endl;

    // get length of file
    sav_file.seekg(0, sav_file.end);
    file_len = sav_file.tellg();
    sav_file.seekg(0, sav_file.beg);

    // FIXME - I guess tellg isn't ideal?
    if ((unsigned int) file_len == (Ram_Bank_Size * num_ram_banks)) {
      std::cout << "reading .sav file into ram" << std::endl;
      sav_file.read((char *)cart_ram, file_len);
    } else {
      std::cerr << "GBcon: bad len for .sav file" << std::endl;
    }
  } else {
    std::cout << "GBcon: sav file not found" << std::endl;
  }
}

void Cartridge::export_sav(std::string sav_path) {
  std::cout << "GBcon: saving ram" << std::endl;
  std::ofstream ofs;
  int num_bytes;
  ofs.open(sav_path,
           std::ofstream::out | std::ofstream::trunc | std::ios::binary);

  if (ofs.is_open() == false) {
    std::cerr << "GBcon: export_sav: error opening .sav file" << std::endl;
    return;
  }

  num_bytes = num_ram_banks * Ram_Bank_Size;
  ofs.write((char *)cart_ram, num_bytes * sizeof(unsigned char));
}

Cartridge::~Cartridge() {
  if (cart_rom != NULL) {
    delete cart_rom;
  }

  if (cart_ram != NULL) {
    delete cart_ram;
  }
}
