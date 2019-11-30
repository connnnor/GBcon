#pragma once
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include "gbcon.h"

using namespace std;

class Timer {
public:
  unsigned char div = 0;  // increments at 16.384kHz
  unsigned char tima = 0; // increments at TAC rate
  unsigned char tma = 0;  // tma->tima when (tima++) > 0xFF
  unsigned char tac = 0;
  unsigned int prev_ticks = 0;
  unsigned char sum_ticks;

  bool timer_en = false;
  unsigned char input_clk = 64;
  unsigned char div_clk = 4;
  unsigned char timer_ticks = 0;

  /* TAC Info
    Bit 2    - Timer Stop  (0=Stop, 1=Start)
    Bits 1-0 - Input Clock Select
               00:   4096 Hz    (~4194 Hz SGB)
               01: 262144 Hz  (~268400 Hz SGB)
               10:  65536 Hz   (~67110 Hz SGB)
               11:  16384 Hz   (~16780 Hz SGB)
  */

  void set_tac(unsigned char value);
  unsigned char get_tac(void);
  // excuted at the rate of the fastest timer (4 machine cycles)
  void increment(void);
  // checks cycles since last timer step. if > 4 cycles, run increment func
  void step(void);

  void init(GB_Sys *gb_sys);

private:
  /* GB system (pointers to other components)
   */

  CPU       *cpu;
  Interrupt *interrupt;
  Memory     *mem;
};
