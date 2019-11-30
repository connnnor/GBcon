#pragma once

class CPU;
class Memory;
class Cartridge;
class LCD;
class Interrupt;
class Timer;
class Debug;

struct GB_Sys {
  CPU       *cpu;
  Memory    *mem;
  Cartridge *cart;
  LCD       *lcd;
  Interrupt *interrupt;
  Timer     *timer;
  Debug     *dbg;
};
