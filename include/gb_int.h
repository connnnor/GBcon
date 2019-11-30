#pragma once
#include "gbcon.h"
#include <fstream>
#include <iomanip>
#include <iostream>

class Interrupt {
public:
  void init(GB_Sys *gb_sys);

  enum interrupts {
    VBLANK = 1 << 0,    // Bit 0
    LCDC_STAT = 1 << 1, // Bit 1
    TIMER_OF = 1 << 2,  // Bit 2
    SIO_TX = 1 << 3,    // Bit 3
    HI_LO = 1 << 4,     // Bit 4
  };

  void step(void);
  unsigned char flags = 0;
  unsigned char en = 0;

  typedef void (Interrupt::*fptr)(void);

  struct interrupt {
    std::string int_str;
    fptr execute;
  };

  bool ime_flag = false;

  // interrupt routines
  void vblank(void);
  void lcdc_stat(void);
  void timer_of(void);
  void sio_tx(void);
  void hi_lo(void);
private:

  /* GB system (pointers to other components)
   */

  Memory *mem;
  CPU *cpu;
};
