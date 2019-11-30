#pragma once
#include <iostream>
#include <iomanip>
#include <fstream>

class MBC;

class Cartridge
{
    public:
        Cartridge(std::string rom_path);
        ~Cartridge();
        unsigned char read_byte(unsigned short address);
        void write_byte(unsigned short address, unsigned char value);
        unsigned char *cart_rom, *cart_ram;
        void export_sav(std::string sav_path);
        void import_sav(std::string path);

        bool loaded;
    private:
        MBC *mbc;

        unsigned short num_ram_banks;


        /* constants */
        const unsigned Max_Rom_Size     = 0x400000;
        const unsigned One_KB           = 1024;
        const unsigned Ram_Bank_Size    = (One_KB * 8);

        /* Cartidge header memory addresses */
        const unsigned short Mbc_type_addr = 0x0147;
        const unsigned short Ram_size_addr = 0x0149;
};
