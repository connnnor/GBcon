#pragma once
#include <iostream>
#include <iomanip>
#include <fstream>

class MBC {
    public:
        MBC(unsigned char *p_ram, unsigned char *p_rom);
        unsigned char read_byte(unsigned short address);
        virtual void write_byte(unsigned short address, unsigned char value) = 0;
    protected:
        unsigned char *rom;
        unsigned char *ram;

        unsigned short curr_rom_bank;
        unsigned short curr_ram_bank;
        bool ram_enable = false;

        typedef enum {
            ROM_BANK = 0x00,
            RAM_BANK = 0x01
        } mbc_mode_t;

        mbc_mode_t mbc_mode;

        const unsigned One_KB           = 1024;
        const unsigned Rom_Bank_Size    = (One_KB * 16);
        const unsigned Ram_Bank_Size    = (One_KB * 8);

};

class NoMBC : public MBC {
    public:
        NoMBC(unsigned char *ram, unsigned char *rom) : MBC(ram, rom) {};
        //unsigned char read_byte(unsigned short address);
        void write_byte(unsigned short address, unsigned char value);
};

class MBC1 : public MBC {
    public:
        MBC1(unsigned char *ram, unsigned char *rom) : MBC(ram, rom) {};
        //unsigned char read_byte(unsigned short address);
        void write_byte(unsigned short address, unsigned char value);
};

//class MBC2 : public MBC {
//    public:
//        unsigned char read_byte(unsigned short address);
//        void write_byte(unsigned short address, unsigned char value);
//};

class MBC3 : public MBC {
    public:
        MBC3(unsigned char *ram, unsigned char *rom) : MBC(ram, rom) {};
        //unsigned char read_byte(unsigned short address);
        void write_byte(unsigned short address, unsigned char value);
};

class MBC5 : public MBC {
    public:
        MBC5(unsigned char *ram, unsigned char *rom) : MBC(ram, rom) {};
        //unsigned char read_byte(unsigned short address);
        void write_byte(unsigned short address, unsigned char value);
};

