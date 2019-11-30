#include "gb_int.h"
#include "gb_cpu.h"
#include "gb_memory.h"

void Interrupt::vblank(void) {
  mem->write_short_to_stack(cpu->registers.sp, cpu->registers.pc);
  cpu->registers.sp -= 2;

  // jump to vblank irq
  cpu->registers.pc = 0x0040;
}
void Interrupt::lcdc_stat(void) {
  mem->write_short_to_stack(cpu->registers.sp, cpu->registers.pc);
  cpu->registers.sp -= 2;

  // jump to lcdc status irq
  cpu->registers.pc = 0x0048;
}
void Interrupt::timer_of(void) {
  mem->write_short_to_stack(cpu->registers.sp, cpu->registers.pc);
  cpu->registers.sp -= 2;

  // jump to timer overflow irq
  cpu->registers.pc = 0x0050;
}
void Interrupt::sio_tx(void) {
  std::cout << "unimplemented sio transfer int" << std::endl;
  cpu->stop = true;

  //    mem->write_short_to_stack(cpu->registers.sp, cpu->registers.pc);
  //    cpu->registers.sp -= 2;
  //
  //    //jump to sio tx/rx irq
  //    cpu->registers.pc = 0x0058;
}
void Interrupt::hi_lo(void) {
  std::cout << "unimplemented hilo int" << std::endl;
  cpu->stop = true;

//  mem->write_short_to_stack(cpu->registers.sp, cpu->registers.pc);
//  cpu->registers.sp -= 2;
//
//  //jump to hilo p10-p13
//  cpu->registers.pc = 0x0060;
}

void Interrupt::step(void) {
  // resume exec if interrupt occurs, even if not enabled or IME is false
  if (flags & (VBLANK | LCDC_STAT | TIMER_OF | SIO_TX | HI_LO)) {
    cpu->halted = false;
  }

  if (ime_flag) {
    // if any enabled interrupts are set
    if (flags & en)
      ime_flag = false;
    // can only process one interrupt at a time
    if (flags & en & VBLANK) { // Priority 1
      flags &= ~VBLANK;
      vblank();
    } else if (flags & en & LCDC_STAT) {
      flags &= ~LCDC_STAT;
      lcdc_stat();
    } else if (flags & en & TIMER_OF) {
      flags &= ~TIMER_OF;
      timer_of();
    } else if (flags & en & SIO_TX) {
      flags &= ~SIO_TX;
      sio_tx();
    } else if (flags & en & HI_LO) {
      flags &= ~HI_LO;
      hi_lo();
    }
  }
}


void Interrupt::init(GB_Sys *gb_sys) {
  mem = gb_sys->mem;
  cpu = gb_sys->cpu;
}
