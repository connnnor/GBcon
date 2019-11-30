#include "gb_timer.h"
#include "gb_cpu.h"
#include "gb_sdl.h"
#include "gb_int.h"

unsigned char Timer::get_tac(void) {
  unsigned char tac_byte = 0;

  if (timer_en)
    tac_byte |= 0x04;

  switch (input_clk) {
  case 64:
    tac_byte |= 0x00;
    break;
  case 1:
    tac_byte |= 0x01;
    break;
  case 16:
    tac_byte |= 0x02;
    break;
  case 4:
    tac_byte |= 0x03;
    break;
  }

  return tac_byte;
}
void Timer::set_tac(unsigned char value) {
  timer_en = (value & 0x04);

  switch (value & 0x03) {
  case 0x0:
    input_clk = 64;
    break;
  case 0x1:
    input_clk = 1;
    break;
  case 0x2:
    input_clk = 16;
    break;
  case 0x3:
    input_clk = 4;
    break;
  }
}

void Timer::increment(void) {
  timer_ticks++;

  // increment div?
  if ((timer_ticks % div_clk) == 0)
    div++;

  if (timer_en == false)
    return;

  // increment tima?
  if ((timer_ticks % input_clk) == 0) {
    if (tima == 0xFF) {
      tima = tma;
      interrupt->flags |= Interrupt::TIMER_OF;
    } else {
      tima++;
    }
  }
}

void Timer::step(void) {
  unsigned int delta_t = cpu->ticks - prev_ticks;

  sum_ticks += delta_t;

  if (sum_ticks > 4) {
    increment();
    sum_ticks -= 4;
  }
  prev_ticks = cpu->ticks;
}

void Timer::init(GB_Sys *gb_sys) {
  cpu = gb_sys->cpu;
  interrupt = gb_sys->interrupt;
}
