#include "gb_cpu.h"
#include "gb_int.h"
#include "gb_memory.h"

using namespace std;

void CPU::reset(void) {

  registers.pc = 0x0000;
  registers.af = (registers.af & 0xFFF0);

  /* initialize io regs */
  mem->write_byte(0xFF40, 0x91);
  mem->write_byte(0xFF41, 0x80);
  mem->write_byte(0xFF42, 0x00);
  mem->write_byte(0xFF43, 0x00);
  // 0xff44 is read only
  mem->write_byte(0xFF45, 0x00);
  // 0xff46 is write only
  mem->write_byte(0xFF47, 0x00);
  mem->write_byte(0xFF48, 0x00);
  mem->write_byte(0xFF49, 0x00);
  mem->write_byte(0xFF4A, 0x00);
  mem->write_byte(0xFF4B, 0x00);

  mem->write_byte(0xFF44, 0x90);
  // hack for gamepad
  mem->write_byte(0xFF00, 0x00);

  mem->write_byte(0xFF47, 0xFC);
  mem->write_byte(0xFF48, 0xFF);
  mem->write_byte(0xFF49, 0xFF);

  machine_cycle_counter = 0;
  ticks = 0;
}

unsigned int CPU::cpu_step(void) {
  unsigned int instCycles;
  ticks += 1;

  prev_pc = registers.pc;
  if (halted == true) {
    instCycles = 4;
    machine_cycle_counter += instCycles;
    return instCycles;
  }

  // read instruction from mem
  curr_inst = mem->read_byte(registers.pc++);

  // execute
  (this->*(instrs[curr_inst].execute))();

  // figure out how many cycles to incr by based on results of last inst
  if (instrs[curr_inst].prog_control_inst) {
    if (instrs[curr_inst].cond_control_inst) {
      // conditional prog control. need to increment cycles depending on if
      // branch taken or not. don't modify pc set handler does that
      if (branch_taken) {
        instCycles = instrs[curr_inst].cycle_duration_l;
      } else {
        instCycles = instrs[curr_inst].cycle_duration_sh;
      }
    } else {
      // prog control instr but non-conditional
      instCycles = instrs[curr_inst].cycle_duration_sh;
    }
  } else {
    // normal inst. increment cycles by constant value. increment pc
    registers.pc += instrs[curr_inst].operand_length;
    instCycles = instrs[curr_inst].cycle_duration_sh;
  }

  if (curr_inst == 0xCB) {
    registers.pc += 1;
  }

  machine_cycle_counter += instCycles;

  return instCycles;
}

unsigned char CPU::get_x(unsigned char opcode) { return (opcode >> 6); }
unsigned char CPU::get_y(unsigned char opcode) {
  return ((opcode & 0x38) >> 3);
}
unsigned char CPU::get_z(unsigned char opcode) {
  return ((opcode & 0x07) >> 0);
}

void CPU::set_zflag(bool val) {
  if (val)
    registers.af = (registers.af & 0xFF7F) | (0x0080);
  else
    registers.af = (registers.af & 0xFF7F);
}

void CPU::set_nflag(bool val) {
  if (val)
    registers.af = (registers.af & 0xFFBF) | (0x0040);
  else
    registers.af = (registers.af & 0xFFBF);
}

void CPU::set_hflag(bool val) {
  if (val)
    registers.af = (registers.af & 0xFFDF) | (0x0020);
  else
    registers.af = (registers.af & 0xFFDF);
}

void CPU::set_cflag(bool val) {
  if (val)
    registers.af = (registers.af & 0xFFEF) | (0x0010);
  else
    registers.af = (registers.af & 0xFFEF);
}

bool CPU::check_zflag(void) { return bool(registers.af & 0x0080); }
bool CPU::check_nflag(void) { return bool(registers.af & 0x0040); }
bool CPU::check_hflag(void) { return bool(registers.af & 0x0020); }
bool CPU::check_cflag(void) { return bool(registers.af & 0x0010); }

void CPU::_unimplemented(void) {
  cout << endl;
  cout << "unimplemented opcode " << hex << unsigned(curr_inst) << endl;
  cout << instrs[curr_inst].disassembly << endl;
  cout << endl;
  stop = true;
}
// 0x00
void CPU::nop(void) { ; }

void CPU::LD_BC_d16(void) { // 0x01
  unsigned short operand;

  operand = mem->read_short(registers.pc);
  registers.bc = operand;
}

void CPU::LD_BC_A(void) { // 0x02
  unsigned char temp_reg;
  temp_reg = ((registers.af >> 8) & 0x00FF);
  mem->write_byte(registers.bc, temp_reg);
}
void CPU::INC_BC(void) { // 0x03
  registers.bc++;
}

void CPU::INC_B(void) { // 0x04
  unsigned char temp_reg, hcarry_test;

  temp_reg = ((registers.bc >> 8) & 0x00FF);
  hcarry_test = (temp_reg & 0x0F) + 1;
  temp_reg++;

  set_zflag(temp_reg == 0x00);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);

  registers.bc = ((temp_reg << 8) & 0xFF00) | (registers.bc & 0x00FF);
}
void CPU::DEC_B(void) { // 0x05
  unsigned char temp_reg, hcarry_test;

  temp_reg = ((registers.bc >> 8) & 0x00FF);
  hcarry_test = (temp_reg & 0x0F) - 1;
  temp_reg--;

  set_zflag(temp_reg == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);

  registers.bc = ((temp_reg << 8) & 0xFF00) | (registers.bc & 0x00FF);
}
void CPU::LD_B_d8(void) { // 0x06
  unsigned char operand;

  operand = mem->read_byte(registers.pc);
  registers.bc = ((operand << 8) & 0xFF00) | (registers.bc & 0x00FF);
}
void CPU::RLCA(void) { // 0x07
  // basically all documentation for this inst is wrong. always reset zflag
  unsigned char temp_reg_a;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);

  temp_reg_a = ((temp_reg_a >> 7) & 0x01) | ((temp_reg_a << 1) & 0xFE);

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(false);
  set_nflag(false);
  set_hflag(false);
  set_cflag(temp_reg_a & 0x01);
}
void CPU::LD_a16_SP(void) { // 0x08
  unsigned short operand;

  operand = mem->read_short(registers.pc);
  mem->write_short(operand, registers.sp);
}
void CPU::ADD_HL_BC(void) { // 0x09
  unsigned carry_test;
  unsigned short hcarry_test;

  carry_test = registers.hl + registers.bc;
  hcarry_test = ((registers.hl & 0x0FFF) + (registers.bc & 0x0FFF));
  registers.hl += registers.bc;

  set_cflag(carry_test & 0x00010000);
  set_hflag(hcarry_test >= 0x1000);
  set_nflag(false);
}
void CPU::LD_A_BC(void) { // 0x0a
  unsigned char read_val;

  read_val = mem->read_byte(registers.bc);

  registers.af = ((read_val << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::DEC_BC(void) { // 0x0b
  registers.bc--;
}

void CPU::INC_C(void) { // 0x0c
  unsigned char temp_reg_c, hcarry_test;

  temp_reg_c = ((registers.bc >> 0) & 0x00FF);
  hcarry_test = (temp_reg_c & 0x0F) + 1;
  temp_reg_c++;

  registers.bc = (registers.bc & 0xFF00) | (temp_reg_c & 0x00FF);

  set_zflag(temp_reg_c == 0x00);
  set_hflag(hcarry_test >= 0x10);
  set_nflag(false);
}
void CPU::DEC_C(void) { // 0x0d
  unsigned char temp_reg, hcarry_test;
  temp_reg = ((registers.bc >> 0) & 0x00FF);
  hcarry_test = (temp_reg & 0x0F) - 1;
  temp_reg--;

  set_zflag(temp_reg == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);

  registers.bc = ((registers.bc & 0xFF00) | (temp_reg & 0x00FF));
}
void CPU::LD_C_d8(void) { // 0x0e
  unsigned char operand;
  operand = mem->read_byte(registers.pc);
  registers.bc = (registers.bc & 0xFF00) | (operand & 0x00FF);
}
void CPU::RRCA(void) { // 0x0f
  // basically all documentation for this inst is wrong. always reset zflag
  unsigned char temp_reg;
  temp_reg = ((registers.af >> 8) & 0x00FF);
  temp_reg = ((temp_reg >> 1) & 0x7F) | ((temp_reg << 7) & 0x80);

  set_cflag(registers.af & 0x0100);

  registers.af = ((temp_reg << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_nflag(false);
  set_hflag(false);
  set_zflag(false);
}
void CPU::STOP_0(void) { // 0x10
  ;
}
void CPU::LD_DE_d16(void) { // 0x11
  // 16bit imm -> DE
  registers.de = mem->read_short(registers.pc);
}
void CPU::LD_DE_A(void) { // 0x12
  unsigned char temp_reg_a;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  mem->write_byte(registers.de, temp_reg_a);
}
void CPU::INC_DE(void) { // 0x13
  registers.de++;
}
void CPU::INC_D(void) { // 0x14
  unsigned char temp_reg_d, hcarry_test;

  temp_reg_d = ((registers.de >> 8) & 0x00FF);
  hcarry_test = (temp_reg_d & 0x0F) + 1;
  temp_reg_d++;

  registers.de = ((temp_reg_d << 8) & 0xFF00) | (registers.de & 0x00FF);

  set_zflag(temp_reg_d == 0x00);
  set_hflag(hcarry_test >= 0x10);
  set_nflag(false);
}
void CPU::DEC_D(void) { // 0x15
  unsigned char temp_reg, hcarry_test;

  temp_reg = ((registers.de >> 8) & 0x00FF);
  hcarry_test = (temp_reg & 0x0F) - 1;
  temp_reg--;

  set_zflag(temp_reg == 0x000);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);

  registers.de = ((temp_reg << 8) & 0xFF00) | (registers.de & 0x00FF);
}
void CPU::LD_D_d8(void) { // 0x16
  unsigned char operand;
  operand = mem->read_byte(registers.pc);
  registers.de = ((operand << 8) & 0xFF00) | (registers.de & 0x00FF);
}
void CPU::RLA(void) { // 0x17
  // basically all documentation for this inst is wrong. always reset zflag
  unsigned char temp_reg;

  temp_reg = ((registers.af >> 8) & 0x00FF);
  temp_reg = ((temp_reg << 1) & 0xFE);
  if (check_cflag())
    temp_reg = (temp_reg | 0x01);

  set_cflag(registers.af & 0x8000);

  registers.af = ((temp_reg << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_nflag(false);
  set_hflag(false);
  set_zflag(false);
}
void CPU::JR_r8(void) { // 0x18
  registers.pc += (signed char)mem->read_byte(registers.pc) + 1;
}
void CPU::ADD_HL_DE(void) { // 0x19
  unsigned carry_test;
  unsigned short hcarry_test;

  carry_test = registers.hl + registers.de;
  hcarry_test = ((registers.hl & 0x0FFF) + (registers.de & 0x0FFF));
  registers.hl += registers.de;

  set_cflag(carry_test & 0x00010000);
  set_hflag(hcarry_test >= 0x1000);
  set_nflag(false);
}
void CPU::LD_A_DE(void) { // 0x1a
  unsigned char operand;

  operand = mem->read_byte(registers.de);
  registers.af = (registers.af & 0x00FF) | ((operand << 8) & 0xFF00);
}
void CPU::DEC_DE(void) { // 0x1b
  registers.de--;
}
void CPU::INC_E(void) { // 0x1c
  unsigned char temp_reg_e, hcarry_test;

  temp_reg_e = ((registers.de >> 0) & 0x00FF);
  hcarry_test = (temp_reg_e & 0x0F) + 1;
  temp_reg_e++;

  registers.de = (registers.de & 0xFF00) | (temp_reg_e & 0x00FF);

  set_zflag(temp_reg_e == 0x00);
  set_hflag(hcarry_test >= 0x10);
  set_nflag(false);
}
void CPU::DEC_E(void) { // 0x1d
  unsigned char temp_reg, hcarry_test;
  temp_reg = ((registers.de >> 0) & 0x00FF);
  hcarry_test = (temp_reg & 0x0F) - 1;
  temp_reg--;

  set_zflag(temp_reg == 0x000);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);

  registers.de = ((registers.de & 0xFF00) | (temp_reg & 0x00FF));
}
void CPU::LD_E_d8(void) { // 0x1e
  unsigned char operand;

  operand = mem->read_byte(registers.pc);
  registers.de = (registers.de & 0xFF00) | (operand & 0x00FF);
}
void CPU::RRA(void) { // 0x1f
  // basically all documentation for this inst is wrong. always reset zflag
  unsigned char temp_reg;
  temp_reg = ((registers.af >> 8) & 0x00FF);
  temp_reg = ((temp_reg >> 1) & 0x7F);

  if (check_cflag() == true)
    temp_reg |= 0x80;

  set_cflag(registers.af & 0x0100);

  registers.af = ((temp_reg << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_nflag(false);
  set_hflag(false);
  set_zflag(false);
}
void CPU::JR_NZ_r8(void) { // 0x20
  if (check_zflag() == false) {
    branch_taken = true;
    registers.pc += (signed char)mem->read_byte(registers.pc) + 1;
  } else {
    branch_taken = false;
    registers.pc++;
  }
}
void CPU::LD_HL_d16(void) { // 0x21
  registers.hl = mem->read_short(registers.pc);
}
void CPU::LD_HLp_A(void) { // 0x22
  unsigned char operand;

  operand = ((registers.af >> 8) & 0x00FF);
  mem->write_byte(registers.hl, operand);
  registers.hl++;
}
void CPU::INC_HL(void) { // 0x23
  registers.hl++;
}
void CPU::INC_H(void) { // 0x24
  unsigned char temp_reg, hcarry_test;

  temp_reg = ((registers.hl >> 8) & 0x00FF);
  hcarry_test = (temp_reg & 0x0F) + 1;
  temp_reg++;

  set_zflag(temp_reg == 0x00);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);

  registers.hl = ((temp_reg << 8) & 0xFF00) | (registers.hl & 0x00FF);
}
void CPU::DEC_H(void) { // 0x25
  unsigned char temp_reg_h, hcarry_test;

  temp_reg_h = ((registers.hl >> 8) & 0x00FF);
  hcarry_test = (temp_reg_h & 0x0F) - 1;
  temp_reg_h--;

  registers.hl = ((temp_reg_h << 8) & 0xFF00) | (registers.hl & 0x00FF);

  set_zflag(temp_reg_h == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);
}
void CPU::LD_H_d8(void) { // 0x26
  unsigned char operand;

  operand = mem->read_byte(registers.pc);
  registers.hl = ((operand << 8) & 0xFF00) | (registers.hl & 0x00FF);
}
void CPU::DAA(void) { // 0x27
  int16_t result;

  result = (registers.af >> 8);
  registers.af &= ~(0xFF00 | 0x0080); // reset A and zflag

  if (check_nflag()) { // subtraction
    if (check_hflag()) {
      result = (result - 0x06) & 0xFF;
    }

    if (check_cflag()) {
      result -= 0x60;
    }
  } else { // addition
    if (check_hflag() || (result & 0x0F) > 0x09) {
      result += 0x06;
    }

    if (check_cflag() || result > 0x9F) {
      result += 0x60;
    }
  }

  set_zflag((result & 0xFF) == 0x00);
  set_hflag(false);

  if ((result & 0x100) == 0x100) {
    set_cflag(true);
  }


  registers.af = ((result << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::JR_Z_r8(void) { // 0x28
  if (check_zflag() == true) {
    branch_taken = true;
    registers.pc += (signed char)mem->read_byte(registers.pc) + 1;
  } else {
    branch_taken = false;
    registers.pc++;
  }
}
void CPU::ADD_HL_HL(void) { // 0x29
  unsigned carry_test;
  unsigned short hcarry_test;

  carry_test = registers.hl + registers.hl;
  hcarry_test = ((registers.hl & 0x0FFF) + (registers.hl & 0x0FFF));
  registers.hl += registers.hl;

  set_nflag(false);
  set_hflag(hcarry_test >= 0x1000);
  set_cflag(carry_test & 0x00010000);
}
void CPU::LD_A_HL_pl(void) { // 0x2a
  unsigned char read_val;

  read_val = mem->read_byte(registers.hl);

  registers.af = ((read_val << 8) & 0xFF00) | (registers.af & 0x00FF);
  registers.hl++;
}
void CPU::DEC_HL(void) { // 0x2b
  registers.hl--;
}
void CPU::INC_L(void) { // 0x2c
  unsigned char temp_reg, hcarry_test;

  temp_reg = ((registers.hl >> 0) & 0x00FF);
  hcarry_test = (temp_reg & 0x0F) + 1;
  temp_reg++;

  set_zflag(temp_reg == 0x00);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);

  registers.hl = (registers.hl & 0xFF00) | (temp_reg & 0x00FF);
}
void CPU::DEC_L(void) { // 0x2d
  unsigned char temp_reg, hcarry_test;
  temp_reg = ((registers.hl >> 0) & 0x00FF);
  hcarry_test = (temp_reg & 0x0F) - 1;
  temp_reg--;

  set_zflag(temp_reg == 0x000);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);

  registers.hl = (registers.hl & 0xFF00) | (temp_reg & 0x00FF);
}
void CPU::LD_L_d8(void) { // 0x2e
  unsigned char operand;
  operand = mem->read_byte(registers.pc);
  registers.hl = (registers.hl & 0xFF00) | (operand & 0x00FF);
}
void CPU::CPL(void) { // 0x2f
  unsigned char temp_reg_a;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_a ^= 0xFF;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_nflag(true);
  set_hflag(true);
}
void CPU::JR_NC_r8(void) { // 0x30
  if (check_cflag() == false) {
    registers.pc += (signed char)mem->read_byte(registers.pc) + 1;
  } else {
    registers.pc++;
  }
}
void CPU::LD_SP_d16(void) { // 0x31
  // Put operand (2 bytes) into SP
  registers.sp = mem->read_short(registers.pc);
}
void CPU::LD_HLm_A(void) { // 0x32
  // *(registers.hl) = registers.a and decrement hl
  mem->write_byte(registers.hl, (registers.af >> 8));
  registers.hl--;
}
void CPU::INC_SP(void) { // 0x33
  registers.sp++;
}
void CPU::INC_HL2(void) { // 0x34
  unsigned char temp_reg;
  unsigned char hcarry_test;

  temp_reg = mem->read_byte(registers.hl);
  hcarry_test = (temp_reg & 0x0F) + 1;
  temp_reg++;
  mem->write_byte(registers.hl, temp_reg);

  set_zflag(temp_reg == 0x00);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);
}
void CPU::DEC_HL2(void) { // 0x35
  unsigned char temp_reg;
  unsigned char hcarry_test;

  temp_reg = mem->read_byte(registers.hl);
  hcarry_test = (temp_reg & 0x0F) - 1;
  temp_reg--;
  mem->write_byte(registers.hl, temp_reg);

  set_zflag(temp_reg == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);
}
void CPU::LD_HL_d8(void) { // 0x36
  unsigned char operand;

  operand = mem->read_byte(registers.pc);
  mem->write_byte(registers.hl, operand);
}
void CPU::SCF(void) { // 0x37
  set_nflag(false);
  set_hflag(false);
  set_cflag(true);
}
void CPU::JR_C_r8(void) { // 0x38
  if (check_cflag() == true) {
    branch_taken = true;
    registers.pc += (signed char)mem->read_byte(registers.pc) + 1;
  } else {
    branch_taken = false;
    registers.pc++;
  }
}
void CPU::ADD_HL_SP(void) { // 0x39
  unsigned carry_test;
  unsigned char hcarry_test;

  carry_test = registers.hl + registers.sp;
  hcarry_test = (((registers.hl & 0x0F00) + (registers.sp & 0x0F00)) >> 8);
  if (((registers.hl & 0x00FF) + (registers.sp & 0x00FF)) >= 0x0100)
    hcarry_test += 1;

  registers.hl += registers.sp;

  set_cflag(carry_test & 0x00010000);
  set_hflag(hcarry_test & 0x10);
  set_nflag(false);
}
void CPU::LD_A_HL_min(void) { // 0x3a
  unsigned char read_val;

  read_val = mem->read_byte(registers.hl);

  registers.af = ((read_val << 8) & 0xFF00) | (registers.af & 0x00FF);
  registers.hl--;
}
void CPU::DEC_SP(void) { // 0x3b
  registers.sp--;
}
void CPU::INC_A(void) { // 0x3c
  unsigned char temp_reg, hcarry_test;

  temp_reg = ((registers.af >> 8) & 0x00FF);
  hcarry_test = (temp_reg & 0x0F) + 1;
  temp_reg++;

  set_zflag(temp_reg == 0x00);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);

  registers.af = ((temp_reg << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::DEC_A(void) { // 0x3d
  unsigned char temp_reg, hcarry_test;
  temp_reg = ((registers.af >> 8) & 0x00FF);
  hcarry_test = (temp_reg & 0x0F) - 1;
  temp_reg--;

  set_zflag(temp_reg == 0x000);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);

  registers.af = ((temp_reg << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::LD_A_d8(void) { // 0x3e
  unsigned char operand;
  operand = mem->read_byte(registers.pc);
  registers.af = ((operand << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::CCF(void) { // 0x3f
  set_cflag(!check_cflag());
  set_nflag(false);
  set_hflag(false);
}
void CPU::LD_B_B(void) { // 0x40
  ;
}
void CPU::LD_B_C(void) { // 0x41
  unsigned char operand;

  operand = ((registers.bc >> 0) & 0x00FF);
  registers.bc = ((operand << 8) & 0xFF00) | (registers.bc & 0x00FF);
}
void CPU::LD_B_D(void) { // 0x42
  unsigned char operand;

  operand = ((registers.de >> 8) & 0x00FF);
  registers.bc = ((operand << 8) & 0xFF00) | (registers.bc & 0x00FF);
}
void CPU::LD_B_E(void) { // 0x43
  unsigned char operand;

  operand = ((registers.de >> 0) & 0x00FF);
  registers.bc = ((operand << 8) & 0xFF00) | (registers.bc & 0x00FF);
}
void CPU::LD_B_H(void) { // 0x44
  unsigned char operand;

  operand = ((registers.hl >> 8) & 0x00FF);
  registers.bc = ((operand << 8) & 0xFF00) | (registers.bc & 0x00FF);
}
void CPU::LD_B_L(void) { // 0x45
  unsigned char operand;

  operand = ((registers.hl >> 0) & 0x00FF);
  registers.bc = ((operand << 8) & 0xFF00) | (registers.bc & 0x00FF);
}
void CPU::LD_B_HL(void) { // 0x46
  unsigned char temp_reg;

  temp_reg = mem->read_byte(registers.hl);
  registers.bc = ((temp_reg << 8) & 0xFF00) | (registers.bc & 0x00FF);
}

void CPU::LD_B_A(void) { // 0x47
  unsigned char temp_reg_a;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  registers.bc = ((temp_reg_a << 8) & 0xFF00) | (registers.bc & 0x00FF);
}
void CPU::LD_C_B(void) { // 0x48
  unsigned char operand;

  operand = ((registers.bc >> 8) & 0x00FF);
  registers.bc = (registers.bc & 0xFF00) | (operand & 0x00FF);
}
void CPU::LD_C_C(void) { // 0x49
  ;
}
void CPU::LD_C_D(void) { // 0x4a
  unsigned char operand;

  operand = ((registers.de >> 8) & 0x00FF);
  registers.bc = (registers.bc & 0xFF00) | (operand & 0x00FF);
}
void CPU::LD_C_E(void) { // 0x4b
  unsigned char operand;

  operand = ((registers.de >> 0) & 0x00FF);
  registers.bc = (registers.bc & 0xFF00) | (operand & 0x00FF);
}
void CPU::LD_C_H(void) { // 0x4c
  unsigned char operand;

  operand = ((registers.hl >> 8) & 0x00FF);
  registers.bc = (registers.bc & 0xFF00) | (operand & 0x00FF);
}
void CPU::LD_C_L(void) { // 0x4d
  unsigned char operand;

  operand = ((registers.hl >> 0) & 0x00FF);
  registers.bc = (registers.bc & 0xFF00) | (operand & 0x00FF);
}
void CPU::LD_C_HL(void) { // 0x4e
  unsigned char operand;

  operand = mem->read_byte(registers.hl);
  registers.bc = (registers.bc & 0xFF00) | ((operand << 0) & 0x00FF);
}
void CPU::LD_C_A(void) { // 0x4f
  unsigned char operand;

  operand = ((registers.af >> 8) & 0x00FF);
  registers.bc = ((registers.bc & 0xFF00) | (operand & 0x00FF));
}
void CPU::LD_D_B(void) { // 0x50
  unsigned char operand;

  operand = ((registers.bc >> 8) & 0x00FF);
  registers.de = ((operand << 8) & 0xFF00) | (registers.de & 0x00FF);
}
void CPU::LD_D_C(void) { // 0x51
  unsigned char operand;

  operand = ((registers.bc >> 0) & 0x00FF);
  registers.de = ((operand << 8) & 0xFF00) | (registers.de & 0x00FF);
}
void CPU::LD_D_D(void) { // 0x52
  ;
}
void CPU::LD_D_E(void) { // 0x53
  unsigned char operand;

  operand = ((registers.de >> 0) & 0x00FF);
  registers.de = ((operand << 8) & 0xFF00) | (registers.de & 0x00FF);
}
void CPU::LD_D_H(void) { // 0x54
  unsigned char operand;

  operand = ((registers.hl >> 8) & 0x00FF);
  registers.de = ((operand << 8) & 0xFF00) | (registers.de & 0x00FF);
}
void CPU::LD_D_L(void) { // 0x55
  unsigned char operand;

  operand = ((registers.hl >> 0) & 0x00FF);
  registers.de = ((operand << 8) & 0xFF00) | (registers.de & 0x00FF);
}
void CPU::LD_D_HL(void) { // 0x56
  unsigned char temp_reg;

  temp_reg = mem->read_byte(registers.hl);
  registers.de = ((temp_reg << 8) & 0xFF00) | (registers.de & 0x00FF);
}

void CPU::LD_D_A(void) { // 0x57
  unsigned char temp_reg;

  temp_reg = ((registers.af >> 8) & 0x00FF);
  registers.de = ((temp_reg << 8) & 0xFF00) | (registers.de & 0x00FF);
}
void CPU::LD_E_B(void) { // 0x58
  unsigned char temp_reg;

  temp_reg = ((registers.bc >> 8) & 0x00FF);
  registers.de = (registers.de & 0xFF00) | (temp_reg & 0x00FF);
}
void CPU::LD_E_C(void) { // 0x59
  unsigned char temp_reg;

  temp_reg = ((registers.bc >> 0) & 0x00FF);
  registers.de = (registers.de & 0xFF00) | (temp_reg & 0x00FF);
}
void CPU::LD_E_D(void) { // 0x5a
  unsigned char temp_reg;

  temp_reg = ((registers.de >> 8) & 0x00FF);
  registers.de = (registers.de & 0xFF00) | (temp_reg & 0x00FF);
}
void CPU::LD_E_E(void) { // 0x5b
  ;
}
void CPU::LD_E_H(void) { // 0x5c
  unsigned char temp_reg;

  temp_reg = ((registers.hl >> 8) & 0x00FF);
  registers.de = (registers.de & 0xFF00) | (temp_reg & 0x00FF);
}
void CPU::LD_E_L(void) { // 0x5d
  unsigned char temp_reg;

  temp_reg = ((registers.hl >> 0) & 0x00FF);
  registers.de = (registers.de & 0xFF00) | (temp_reg & 0x00FF);
}
void CPU::LD_E_HL(void) { // 0x5e
  unsigned char temp_reg;

  temp_reg = mem->read_byte(registers.hl);
  registers.de = (registers.de & 0xFF00) | (temp_reg & 0x00FF);
}
void CPU::LD_E_A(void) { // 0x5f
  unsigned char temp_reg;

  temp_reg = ((registers.af >> 8) & 0x00FF);
  registers.de = (registers.de & 0xFF00) | (temp_reg & 0x00FF);
}
void CPU::LD_H_B(void) { // 0x60
  unsigned char temp_reg_b;

  temp_reg_b = ((registers.bc >> 8) & 0x00FF);
  registers.hl = ((temp_reg_b << 8) & 0xFF00) | (registers.hl & 0x00FF);
}
void CPU::LD_H_C(void) { // 0x61
  unsigned char temp_reg_c;

  temp_reg_c = ((registers.bc >> 0) & 0x00FF);
  registers.hl = ((temp_reg_c << 8) & 0xFF00) | (registers.hl & 0x00FF);
}
void CPU::LD_H_D(void) { // 0x62
  unsigned char temp_reg;

  temp_reg = ((registers.de >> 8) & 0x00FF);
  registers.hl = ((temp_reg << 8) & 0xFF00) | (registers.hl & 0x00FF);
}
void CPU::LD_H_E(void) { // 0x63
  unsigned char temp_reg;

  temp_reg = ((registers.de >> 0) & 0x00FF);
  registers.hl = ((temp_reg << 8) & 0xFF00) | (registers.hl & 0x00FF);
}
void CPU::LD_H_H(void) { // 0x64
  ;
}
void CPU::LD_H_L(void) { // 0x65
  unsigned char temp_reg;

  temp_reg = ((registers.hl >> 0) & 0x00FF);
  registers.hl = ((temp_reg << 8) & 0xFF00) | (registers.hl & 0x00FF);
}
void CPU::LD_H_HL(void) { // 0x66
  unsigned char operand;

  operand = mem->read_byte(registers.hl);
  registers.hl = ((operand << 8) & 0xFF00) | (registers.hl & 0x00FF);
}
void CPU::LD_H_A(void) { // 0x67
  unsigned char temp_reg;

  temp_reg = ((registers.af >> 8) & 0x00FF);
  registers.hl = ((temp_reg << 8) & 0xFF00) | (registers.hl & 0x00FF);
}
void CPU::LD_L_B(void) { // 0x68
  unsigned char temp_reg;

  temp_reg = ((registers.bc >> 8) & 0x00FF);
  registers.hl = (registers.hl & 0xFF00) | (temp_reg & 0x00FF);
}
void CPU::LD_L_C(void) { // 0x69
  unsigned char temp_reg;

  temp_reg = ((registers.bc >> 0) & 0x00FF);
  registers.hl = (registers.hl & 0xFF00) | (temp_reg & 0x00FF);
}

void CPU::LD_L_D(void) { // 0x6a
  unsigned char temp_reg;

  temp_reg = ((registers.de >> 8) & 0x00FF);
  registers.hl = (registers.hl & 0xFF00) | (temp_reg & 0x00FF);
}
void CPU::LD_L_E(void) { // 0x6b
  unsigned char temp_reg;

  temp_reg = ((registers.de >> 0) & 0x00FF);
  registers.hl = (registers.hl & 0xFF00) | (temp_reg & 0x00FF);
}
void CPU::LD_L_H(void) { // 0x6c
  unsigned char temp_reg;

  temp_reg = ((registers.hl >> 8) & 0x00FF);
  registers.hl = (registers.hl & 0xFF00) | (temp_reg & 0x00FF);
}
void CPU::LD_L_L(void) { // 0x6d
  unsigned char temp_reg;

  temp_reg = ((registers.hl >> 0) & 0x00FF);
  registers.hl = (registers.hl & 0xFF00) | (temp_reg & 0x00FF);
}
void CPU::LD_L_HL(void) { // 0x6e
  unsigned char operand;

  operand = mem->read_byte(registers.hl);
  registers.hl = (registers.hl & 0xFF00) | (operand & 0x00FF);
}
void CPU::LD_L_A(void) { // 0x6f
  unsigned char temp_reg_a;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  registers.hl = (registers.hl & 0xFF00) | (temp_reg_a & 0x00FF);
}
void CPU::LD_HL_B(void) { // 0x70
  unsigned char temp_reg_b;

  temp_reg_b = ((registers.bc >> 8) & 0x00FF);
  mem->write_byte(registers.hl, temp_reg_b);
}
void CPU::LD_HL_C(void) { // 0x71
  unsigned char temp_reg_c;

  temp_reg_c = (registers.bc & 0x00FF);
  mem->write_byte(registers.hl, temp_reg_c);
}
void CPU::LD_HL_D(void) { // 0x72
  unsigned char temp_reg_d;

  temp_reg_d = ((registers.de >> 8) & 0x00FF);
  mem->write_byte(registers.hl, temp_reg_d);
}
void CPU::LD_HL_E(void) { // 0x73
  unsigned char temp_reg_e;

  temp_reg_e = (registers.de & 0x00FF);
  mem->write_byte(registers.hl, temp_reg_e);
}
void CPU::LD_HL_H(void) { // 0x74
  unsigned char temp_reg;

  temp_reg = ((registers.hl >> 8) & 0x00FF);
  mem->write_byte(registers.hl, temp_reg);
}
void CPU::LD_HL_L(void) { // 0x75
  unsigned char temp_reg;

  temp_reg = ((registers.hl >> 0) & 0x00FF);
  mem->write_byte(registers.hl, temp_reg);
}
void CPU::HALT(void) { // 0x76
  halted = true;
}
void CPU::LD_HL_A(void) { // 0x77
  unsigned char temp_reg_a;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  mem->write_byte(registers.hl, temp_reg_a);
}
void CPU::LD_A_B(void) { // 0x78
  unsigned char temp_reg;

  temp_reg = ((registers.bc >> 8) & 0x00FF);
  registers.af = ((temp_reg << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::LD_A_C(void) { // 0x79
  unsigned char temp_reg;

  temp_reg = ((registers.bc >> 0) & 0x00FF);
  registers.af = ((temp_reg << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::LD_A_D(void) { // 0x7a
  unsigned char temp_reg;

  temp_reg = ((registers.de >> 8) & 0x00FF);
  registers.af = ((temp_reg << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::LD_A_E(void) { // 0x7b
  unsigned char temp_reg;

  temp_reg = ((registers.de >> 0) & 0x00FF);
  registers.af = ((temp_reg << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::LD_A_H(void) { // 0x7c
  unsigned char temp_reg;

  temp_reg = ((registers.hl >> 8) & 0x00FF);
  registers.af = ((temp_reg << 8) & 0xFF00) | (registers.af & 0x00FF);
}

void CPU::LD_A_L(void) { // 0x7d
  unsigned char temp_reg;

  temp_reg = ((registers.hl >> 0) & 0x00FF);
  registers.af = ((temp_reg << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::LD_A_HL(void) { // 0x7e
  unsigned char operand;

  operand = mem->read_byte(registers.hl);
  registers.af = ((operand << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::LD_A_A(void) { // 0x7f
  ;
}
void CPU::ADD_A_B(void) { // 0x80
  unsigned short carry_test;
  unsigned char temp_reg_a, temp_reg_b, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_b = ((registers.bc >> 8) & 0x00FF);

  hcarry_test = (temp_reg_a & 0x0F) + (temp_reg_b & 0x0F);
  carry_test = temp_reg_a + temp_reg_b;
  temp_reg_a = temp_reg_a + temp_reg_b;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(carry_test & 0x0100);
}
void CPU::ADD_A_C(void) { // 0x81
  unsigned short carry_test;
  unsigned char temp_reg_a, temp_reg_c, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_c = ((registers.bc >> 0) & 0x00FF);

  hcarry_test = (temp_reg_a & 0x0F) + (temp_reg_c & 0x0F);
  carry_test = temp_reg_a + temp_reg_c;
  temp_reg_a = temp_reg_a + temp_reg_c;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(carry_test & 0x0100);
}
void CPU::ADD_A_D(void) { // 0x82
  unsigned short carry_test;
  unsigned char temp_reg_a, temp_reg_d, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_d = ((registers.de >> 8) & 0x00FF);

  hcarry_test = (temp_reg_a & 0x0F) + (temp_reg_d & 0x0F);
  carry_test = temp_reg_a + temp_reg_d;
  temp_reg_a = temp_reg_a + temp_reg_d;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(carry_test & 0x0100);
}

void CPU::ADD_A_E(void) { // 0x83
  unsigned short carry_test;
  unsigned char temp_reg_a, temp_reg_e, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_e = ((registers.de >> 0) & 0x00FF);

  hcarry_test = (temp_reg_a & 0x0F) + (temp_reg_e & 0x0F);
  carry_test = temp_reg_a + temp_reg_e;
  temp_reg_a = temp_reg_a + temp_reg_e;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(carry_test & 0x0100);
}
void CPU::ADD_A_H(void) { // 0x84
  unsigned short carry_test;
  unsigned char temp_reg_a, temp_reg_h, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_h = ((registers.hl >> 8) & 0x00FF);

  hcarry_test = (temp_reg_a & 0x0F) + (temp_reg_h & 0x0F);
  carry_test = temp_reg_a + temp_reg_h;
  temp_reg_a = temp_reg_a + temp_reg_h;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(carry_test & 0x0100);
}
void CPU::ADD_A_L(void) { // 0x85
  unsigned short carry_test;
  unsigned char temp_reg_a, temp_reg_l, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_l = ((registers.hl >> 0) & 0x00FF);

  hcarry_test = (temp_reg_a & 0x0F) + (temp_reg_l & 0x0F);
  carry_test = temp_reg_a + temp_reg_l;
  temp_reg_a = temp_reg_a + temp_reg_l;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(carry_test & 0x0100);
}
void CPU::ADD_A_HL(void) { // 0x86
  // A = A + *(HL)
  unsigned char temp_reg_a, operand, hcarry_test;
  unsigned short carry_test;

  operand = mem->read_byte(registers.hl);
  temp_reg_a = ((registers.af >> 8) & 0x00FF);

  carry_test = temp_reg_a + operand;
  hcarry_test = (temp_reg_a & 0x0F) + (operand & 0x0F);
  temp_reg_a = temp_reg_a + operand;

  set_zflag(temp_reg_a == 0x00);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(carry_test & 0x0100);

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::ADD_A_A(void) { // 0x87
  unsigned char temp_reg_a, result, hcarry_test;
  unsigned short carry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  result = temp_reg_a + temp_reg_a;
  hcarry_test = (temp_reg_a & 0x0F) + (temp_reg_a & 0x0F);
  carry_test = temp_reg_a + temp_reg_a;

  set_zflag(result == 0x00);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(carry_test & 0x0100);

  registers.af = ((result << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::ADC_A_B(void) { // 0x88
  unsigned short carry_test;
  unsigned char temp_reg_a, temp_reg_b, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_b = ((registers.bc >> 8) & 0x00FF);

  if (check_cflag() == true) {
    hcarry_test = (temp_reg_a & 0x0F) + (temp_reg_b & 0x0F) + 1;
    carry_test = temp_reg_a + temp_reg_b + 1;
    temp_reg_a = temp_reg_a + temp_reg_b + 1;
  } else {
    hcarry_test = (temp_reg_a & 0x0F) + (temp_reg_b & 0x0F);
    carry_test = temp_reg_a + temp_reg_b;
    temp_reg_a = temp_reg_a + temp_reg_b;
  }

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(carry_test >= 0x0100);
}
void CPU::ADC_A_C(void) { // 0x89
  unsigned short carry_test;
  unsigned char temp_reg_a, temp_reg_c, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_c = ((registers.bc >> 0) & 0x00FF);

  if (check_cflag() == true) {
    hcarry_test = (temp_reg_a & 0x0F) + (temp_reg_c & 0x0F) + 1;
    carry_test = temp_reg_a + temp_reg_c + 1;
    temp_reg_a = temp_reg_a + temp_reg_c + 1;
  } else {
    hcarry_test = (temp_reg_a & 0x0F) + (temp_reg_c & 0x0F);
    carry_test = temp_reg_a + temp_reg_c;
    temp_reg_a = temp_reg_a + temp_reg_c;
  }

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(carry_test >= 0x0100);
}
void CPU::ADC_A_D(void) { // 0x8a
  unsigned short carry_test;
  unsigned char temp_reg_a, temp_reg_d, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_d = ((registers.de >> 8) & 0x00FF);

  if (check_cflag() == true) {
    hcarry_test = (temp_reg_a & 0x0F) + (temp_reg_d & 0x0F) + 1;
    carry_test = temp_reg_a + temp_reg_d + 1;
    temp_reg_a = temp_reg_a + temp_reg_d + 1;
  } else {
    hcarry_test = (temp_reg_a & 0x0F) + (temp_reg_d & 0x0F);
    carry_test = temp_reg_a + temp_reg_d;
    temp_reg_a = temp_reg_a + temp_reg_d;
  }

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(carry_test >= 0x0100);
}
void CPU::ADC_A_E(void) { // 0x8b
  unsigned short carry_test;
  unsigned char temp_reg_a, temp_reg_e, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_e = ((registers.de >> 0) & 0x00FF);

  if (check_cflag() == true) {
    hcarry_test = (temp_reg_a & 0x0F) + (temp_reg_e & 0x0F) + 1;
    carry_test = temp_reg_a + temp_reg_e + 1;
    temp_reg_a = temp_reg_a + temp_reg_e + 1;
  } else {
    hcarry_test = (temp_reg_a & 0x0F) + (temp_reg_e & 0x0F);
    carry_test = temp_reg_a + temp_reg_e;
    temp_reg_a = temp_reg_a + temp_reg_e;
  }

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(carry_test >= 0x0100);
}
void CPU::ADC_A_H(void) { // 0x8c
  unsigned short carry_test;
  unsigned char temp_reg_a, temp_reg_h, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_h = ((registers.hl >> 8) & 0x00FF);

  if (check_cflag() == true) {
    hcarry_test = (temp_reg_a & 0x0F) + (temp_reg_h & 0x0F) + 1;
    carry_test = temp_reg_a + temp_reg_h + 1;
    temp_reg_a = temp_reg_a + temp_reg_h + 1;
  } else {
    hcarry_test = (temp_reg_a & 0x0F) + (temp_reg_h & 0x0F);
    carry_test = temp_reg_a + temp_reg_h;
    temp_reg_a = temp_reg_a + temp_reg_h;
  }

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(carry_test >= 0x0100);
}
void CPU::ADC_A_L(void) { // 0x8d
  unsigned short carry_test;
  unsigned char temp_reg_a, temp_reg_l, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_l = ((registers.hl >> 0) & 0x00FF);

  if (check_cflag() == true) {
    hcarry_test = (temp_reg_a & 0x0F) + (temp_reg_l & 0x0F) + 1;
    carry_test = temp_reg_a + temp_reg_l + 1;
    temp_reg_a = temp_reg_a + temp_reg_l + 1;
  } else {
    hcarry_test = (temp_reg_a & 0x0F) + (temp_reg_l & 0x0F);
    carry_test = temp_reg_a + temp_reg_l;
    temp_reg_a = temp_reg_a + temp_reg_l;
  }

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(carry_test >= 0x0100);
}
void CPU::ADC_A_HL(void) { // 0x8e
  unsigned short carry_test;
  unsigned char temp_reg_a, temp_reg, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg = mem->read_byte(registers.hl);

  if (check_cflag() == true) {
    hcarry_test = (temp_reg_a & 0x0F) + (temp_reg & 0x0F) + 1;
    carry_test = temp_reg_a + temp_reg + 1;
    temp_reg_a = temp_reg_a + temp_reg + 1;
  } else {
    hcarry_test = (temp_reg_a & 0x0F) + (temp_reg & 0x0F);
    carry_test = temp_reg_a + temp_reg;
    temp_reg_a = temp_reg_a + temp_reg;
  }

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(carry_test >= 0x0100);
}
void CPU::ADC_A_A(void) { // 0x8f
  unsigned short carry_test;
  unsigned char temp_reg_a, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);

  if (check_cflag() == true) {
    hcarry_test = (temp_reg_a & 0x0F) + (temp_reg_a & 0x0F) + 1;
    carry_test = temp_reg_a + temp_reg_a + 1;
    temp_reg_a = temp_reg_a + temp_reg_a + 1;
  } else {
    hcarry_test = (temp_reg_a & 0x0F) + (temp_reg_a & 0x0F);
    carry_test = temp_reg_a + temp_reg_a;
    temp_reg_a = temp_reg_a + temp_reg_a;
  }

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(carry_test >= 0x0100);
}
void CPU::SUB_B(void) { // 0x90
  unsigned char temp_reg_a, temp_reg_b, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_b = ((registers.bc >> 8) & 0x00FF);
  hcarry_test = (temp_reg_a & 0x0F) - (temp_reg_b & 0x0F);

  set_cflag(temp_reg_a < temp_reg_b);

  temp_reg_a = temp_reg_a - temp_reg_b;

  set_zflag(temp_reg_a == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::SUB_C(void) { // 0x91
  unsigned char temp_reg_a, temp_reg_c, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_c = ((registers.bc >> 0) & 0x00FF);
  hcarry_test = (temp_reg_a & 0x0F) - (temp_reg_c & 0x0F);

  set_cflag(temp_reg_a < temp_reg_c);

  temp_reg_a = temp_reg_a - temp_reg_c;

  set_zflag(temp_reg_a == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::SUB_D(void) { // 0x92
  unsigned char temp_reg_a, temp_reg_d, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_d = ((registers.de >> 8) & 0x00FF);
  hcarry_test = (temp_reg_a & 0x0F) - (temp_reg_d & 0x0F);

  set_cflag(temp_reg_a < temp_reg_d);

  temp_reg_a = temp_reg_a - temp_reg_d;

  set_zflag(temp_reg_a == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::SUB_E(void) { // 0x93
  unsigned char temp_reg_a, temp_reg_e, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_e = ((registers.de >> 0) & 0x00FF);
  hcarry_test = (temp_reg_a & 0x0F) - (temp_reg_e & 0x0F);

  set_cflag(temp_reg_a < temp_reg_e);

  temp_reg_a = temp_reg_a - temp_reg_e;

  set_zflag(temp_reg_a == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::SUB_H(void) { // 0x94
  unsigned char temp_reg_a, temp_reg_h, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_h = ((registers.hl >> 8) & 0x00FF);
  hcarry_test = (temp_reg_a & 0x0F) - (temp_reg_h & 0x0F);

  set_cflag(temp_reg_a < temp_reg_h);

  temp_reg_a = temp_reg_a - temp_reg_h;

  set_zflag(temp_reg_a == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::SUB_L(void) { // 0x95
  unsigned char temp_reg_a, temp_reg_l, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_l = ((registers.hl >> 0) & 0x00FF);
  hcarry_test = (temp_reg_a & 0x0F) - (temp_reg_l & 0x0F);

  set_cflag(temp_reg_a < temp_reg_l);

  temp_reg_a = temp_reg_a - temp_reg_l;

  set_zflag(temp_reg_a == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::SUB_HL(void) { // 0x96
  unsigned char result, operand, temp_reg_a;
  unsigned char hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  operand = mem->read_byte(registers.hl);
  hcarry_test = (temp_reg_a & 0x0F) - (operand & 0x0F);
  result = temp_reg_a - operand;

  set_zflag(result == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(operand > temp_reg_a);

  registers.af = ((result << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::SUB_A(void) { // 0x97
  registers.af = registers.af & 0x00FF;

  set_zflag(true);
  set_nflag(true);
  set_hflag(false);
  set_cflag(false);
}
void CPU::SBC_A_B(void) { // 0x98
  unsigned short temp_reg_a, temp_reg_b, hcarry_test;
  // A = A - (B + cflag)
  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_b = ((registers.bc >> 8) & 0x00FF);
  hcarry_test =
      (temp_reg_a & 0x0F) - (temp_reg_b & 0x0F) - (check_cflag() == true);

  if (check_cflag())
    temp_reg_b++;

  set_cflag(temp_reg_a < temp_reg_b);

  temp_reg_a -= temp_reg_b;

  set_zflag((temp_reg_a & 0x00FF) == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::SBC_A_C(void) { // 0x99
  unsigned short temp_reg_a, temp_reg_c, hcarry_test;
  // A = A - (C + cflag)
  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_c = ((registers.bc >> 0) & 0x00FF);
  hcarry_test =
      (temp_reg_a & 0x0F) - (temp_reg_c & 0x0F) - (check_cflag() == true);

  if (check_cflag())
    temp_reg_c++;

  set_cflag(temp_reg_a < temp_reg_c);

  temp_reg_a -= temp_reg_c;

  set_zflag((temp_reg_a & 0x00FF) == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::SBC_A_D(void) { // 0x9a
  unsigned short temp_reg_a, temp_reg_d, hcarry_test;
  // A = A - (D + cflag)
  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_d = ((registers.de >> 8) & 0x00FF);
  hcarry_test =
      (temp_reg_a & 0x0F) - (temp_reg_d & 0x0F) - (check_cflag() == true);

  if (check_cflag())
    temp_reg_d++;

  set_cflag(temp_reg_a < temp_reg_d);

  temp_reg_a -= temp_reg_d;

  set_zflag((temp_reg_a & 0x00FF) == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::SBC_A_E(void) { // 0x9b
  unsigned short temp_reg_a, temp_reg_e, hcarry_test;
  // A = A - (D + cflag)
  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_e = ((registers.de >> 0) & 0x00FF);
  hcarry_test =
      (temp_reg_a & 0x0F) - (temp_reg_e & 0x0F) - (check_cflag() == true);

  if (check_cflag())
    temp_reg_e++;

  set_cflag(temp_reg_a < temp_reg_e);

  temp_reg_a -= temp_reg_e;

  set_zflag((temp_reg_a & 0x00FF) == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::SBC_A_H(void) { // 0x9c
  unsigned short temp_reg_a, temp_reg_h, hcarry_test;
  // A = A - (H + cflag)
  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_h = ((registers.hl >> 8) & 0x00FF);
  hcarry_test =
      (temp_reg_a & 0x0F) - (temp_reg_h & 0x0F) - (check_cflag() == true);

  if (check_cflag())
    temp_reg_h++;

  set_cflag(temp_reg_a < temp_reg_h);

  temp_reg_a -= temp_reg_h;

  set_zflag((temp_reg_a & 0x00FF) == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::SBC_A_L(void) { // 0x9d
  unsigned short temp_reg_a, temp_reg_l, hcarry_test;
  // A = A - (H + cflag)
  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_l = ((registers.hl >> 0) & 0x00FF);
  hcarry_test =
      (temp_reg_a & 0x0F) - (temp_reg_l & 0x0F) - (check_cflag() == true);

  if (check_cflag())
    temp_reg_l++;

  set_cflag(temp_reg_a < temp_reg_l);

  temp_reg_a -= temp_reg_l;

  set_zflag((temp_reg_a & 0x00FF) == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::SBC_A_HL(void) { // 0x9e
  unsigned short temp_reg_a, temp_reg;
  unsigned char hcarry_test;
  // A = A - (*(hl) + cflag)
  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg = mem->read_byte(registers.hl);

  hcarry_test =
      (temp_reg_a & 0x0F) - (temp_reg & 0x0F) - (check_cflag() == true);

  if (check_cflag())
    temp_reg++;

  set_cflag(temp_reg_a < temp_reg);

  temp_reg_a -= temp_reg;

  set_zflag((temp_reg_a & 0x00FF) == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::SBC_A_A(void) { // 0x9f
  unsigned short temp_reg_a, temp_reg_a2, hcarry_test;
  // A = A - (H + cflag)
  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_a2 = temp_reg_a;
  hcarry_test =
      (temp_reg_a & 0x0F) - (temp_reg_a & 0x0F) - (check_cflag() == true);

  if (check_cflag())
    temp_reg_a2++;

  set_cflag(temp_reg_a < temp_reg_a2);

  temp_reg_a -= temp_reg_a2;

  set_zflag((temp_reg_a & 0x00FF) == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::AND_B(void) { // 0xa0
  unsigned char temp_reg_a, temp_reg_b;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_b = ((registers.bc >> 8) & 0x00FF);
  temp_reg_a &= temp_reg_b;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0x00);
  set_nflag(false);
  set_hflag(true);
  set_cflag(false);
}
void CPU::AND_C(void) { // 0xa1
  unsigned char temp_reg_a, temp_reg_c;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_c = (registers.bc & 0x00FF);
  temp_reg_a &= temp_reg_c;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0x00);
  set_nflag(false);
  set_hflag(true);
  set_cflag(false);
}
void CPU::AND_D(void) { // 0xa2
  unsigned char temp_reg_a, temp_reg_d;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_d = ((registers.de >> 8) & 0x00FF);
  temp_reg_a &= temp_reg_d;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0x00);
  set_nflag(false);
  set_hflag(true);
  set_cflag(false);
}
void CPU::AND_E(void) { // 0xa3
  unsigned char temp_reg_a, temp_reg_e;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_e = ((registers.de >> 0) & 0x00FF);
  temp_reg_a &= temp_reg_e;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0x00);
  set_nflag(false);
  set_hflag(true);
  set_cflag(false);
}
void CPU::AND_H(void) { // 0xa4
  unsigned char temp_reg_a, temp_reg_h;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_h = ((registers.hl >> 8) & 0x00FF);
  temp_reg_a &= temp_reg_h;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0x00);
  set_nflag(false);
  set_hflag(true);
  set_cflag(false);
}
void CPU::AND_L(void) { // 0xa5
  unsigned char temp_reg_a, temp_reg_l;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_l = ((registers.hl >> 0) & 0x00FF);
  temp_reg_a &= temp_reg_l;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0x00);
  set_nflag(false);
  set_hflag(true);
  set_cflag(false);
}
void CPU::AND_HL(void) { // 0xa6
  unsigned char temp_reg_a, temp_reg;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg = mem->read_byte(registers.hl);
  temp_reg_a &= temp_reg;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0x00);
  set_nflag(false);
  set_hflag(true);
  set_cflag(false);
}
void CPU::AND_A(void) { // 0xa7
  unsigned char temp_reg_a;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(true);
  set_cflag(false);
}
void CPU::XOR_B(void) { // 0xa8
  unsigned char temp_reg_a, temp_reg_b;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_b = ((registers.bc >> 8) & 0x00FF);
  temp_reg_a ^= temp_reg_b;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(false);
  set_cflag(false);
}
void CPU::XOR_C(void) { // 0xa9
  unsigned char temp_reg_a, temp_reg_c;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_c = ((registers.bc >> 0) & 0x00FF);
  temp_reg_a ^= temp_reg_c;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(false);
  set_cflag(false);
}
void CPU::XOR_D(void) { // 0xaa
  unsigned char temp_reg_a, temp_reg_d;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_d = ((registers.de >> 8) & 0x00FF);
  temp_reg_a ^= temp_reg_d;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(false);
  set_cflag(false);
}
void CPU::XOR_E(void) { // 0xab
  unsigned char temp_reg_a, temp_reg_e;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_e = ((registers.de >> 0) & 0x00FF);
  temp_reg_a ^= temp_reg_e;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(false);
  set_cflag(false);
}
void CPU::XOR_H(void) { // 0xac
  unsigned char temp_reg_a, temp_reg_h;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_h = ((registers.hl >> 8) & 0x00FF);
  temp_reg_a ^= temp_reg_h;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(false);
  set_cflag(false);
}
void CPU::XOR_L(void) { // 0xad
  unsigned char temp_reg_a, temp_reg_l;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_l = (registers.hl & 0x00FF);
  temp_reg_a ^= temp_reg_l;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(false);
  set_cflag(false);
}
void CPU::XOR_HL(void) { // 0xae
  unsigned char temp_reg_a, temp_reg;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg = mem->read_byte(registers.hl);
  temp_reg_a ^= temp_reg;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0x00);
  set_nflag(false);
  set_hflag(false);
  set_cflag(false);
}
void CPU::XOR_A(void) { // 0xaf
  registers.af = (0x0000 & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(true);
  set_nflag(false);
  set_hflag(false);
  set_cflag(false);
}
void CPU::OR_B(void) { // 0xb0
  unsigned char temp_reg_a, temp_reg_b;

  temp_reg_a = ((registers.af >> 8) & 0xFF);
  temp_reg_b = ((registers.bc >> 8) & 0xFF);

  temp_reg_a |= temp_reg_b;
  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0x00);
  set_nflag(false);
  set_hflag(false);
  set_cflag(false);
}
void CPU::OR_C(void) { // 0xb1
  unsigned char temp_reg_a, temp_reg_c;

  temp_reg_a = ((registers.af >> 8) & 0xFF);
  temp_reg_c = ((registers.bc >> 0) & 0xFF);

  temp_reg_a |= temp_reg_c;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0x00);
  set_nflag(false);
  set_hflag(false);
  set_cflag(false);
}
void CPU::OR_D(void) { // 0xb2
  unsigned char temp_reg_a, temp_reg_d;

  temp_reg_a = ((registers.af >> 8) & 0xFF);
  temp_reg_d = ((registers.de >> 8) & 0xFF);

  temp_reg_a |= temp_reg_d;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0x00);
  set_nflag(false);
  set_hflag(false);
  set_cflag(false);
}
void CPU::OR_E(void) { // 0xb3
  unsigned char temp_reg_a, temp_reg_e;

  temp_reg_a = ((registers.af >> 8) & 0xFF);
  temp_reg_e = ((registers.de >> 0) & 0xFF);

  temp_reg_a |= temp_reg_e;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0x00);
  set_nflag(false);
  set_hflag(false);
  set_cflag(false);
}
void CPU::OR_H(void) { // 0xb4
  unsigned char temp_reg_a, temp_reg_h;

  temp_reg_a = ((registers.af >> 8) & 0xFF);
  temp_reg_h = ((registers.hl >> 8) & 0xFF);

  temp_reg_a |= temp_reg_h;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0x00);
  set_nflag(false);
  set_hflag(false);
  set_cflag(false);
}
void CPU::OR_L(void) { // 0xb5
  unsigned char temp_reg_a, temp_reg_l;

  temp_reg_a = ((registers.af >> 8) & 0xFF);
  temp_reg_l = ((registers.hl >> 0) & 0xFF);

  temp_reg_a |= temp_reg_l;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0x00);
  set_nflag(false);
  set_hflag(false);
  set_cflag(false);
}
void CPU::OR_HL(void) { // 0xb6
  unsigned char temp_reg_a, temp_reg;

  temp_reg_a = ((registers.af >> 8) & 0xFF);
  temp_reg = mem->read_byte(registers.hl);

  temp_reg_a |= temp_reg;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0x00);
  set_nflag(false);
  set_hflag(false);
  set_cflag(false);
}
void CPU::OR_A(void) { // 0xb7
  set_zflag((registers.af & 0xFF00) == 0x0000);
  set_nflag(false);
  set_hflag(false);
  set_cflag(false);
}
void CPU::CP_B(void) { // 0xb8
  unsigned char temp_reg_a, temp_reg_b, result, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_b = ((registers.bc >> 8) & 0x00FF);
  hcarry_test = (temp_reg_a & 0x0F) - (temp_reg_b & 0x0F);
  result = temp_reg_a - temp_reg_b;

  set_zflag(result == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(temp_reg_a < temp_reg_b);
}

void CPU::CP_C(void) { // 0xb9
  unsigned char temp_reg_a, temp_reg_c, result, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_c = ((registers.bc >> 0) & 0x00FF);
  hcarry_test = (temp_reg_a & 0x0F) - (temp_reg_c & 0x0F);
  result = temp_reg_a - temp_reg_c;

  set_zflag(result == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(temp_reg_a < temp_reg_c);
}
void CPU::CP_D(void) { // 0xba
  unsigned char temp_reg_a, temp_reg_d, result, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_d = ((registers.de >> 8) & 0x00FF);
  hcarry_test = (temp_reg_a & 0x0F) - (temp_reg_d & 0x0F);
  result = temp_reg_a - temp_reg_d;

  set_zflag(result == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(temp_reg_a < temp_reg_d);
}
void CPU::CP_E(void) { // 0xbb
  unsigned char temp_reg_a, temp_reg_e, result, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_e = ((registers.de >> 0) & 0x00FF);
  hcarry_test = (temp_reg_a & 0x0F) - (temp_reg_e & 0x0F);
  result = temp_reg_a - temp_reg_e;

  set_zflag(result == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(temp_reg_a < temp_reg_e);
}
void CPU::CP_H(void) { // 0xbc
  unsigned char temp_reg_a, temp_reg_h, result, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_h = ((registers.hl >> 8) & 0x00FF);
  hcarry_test = (temp_reg_a & 0x0F) - (temp_reg_h & 0x0F);
  result = temp_reg_a - temp_reg_h;

  set_zflag(result == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(temp_reg_a < temp_reg_h);
}
void CPU::CP_L(void) { // 0xbd
  unsigned char temp_reg_a, temp_reg_l, result, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg_l = ((registers.hl >> 0) & 0x00FF);
  hcarry_test = (temp_reg_a & 0x0F) - (temp_reg_l & 0x0F);
  result = temp_reg_a - temp_reg_l;

  set_zflag(result == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(temp_reg_a < temp_reg_l);
}
void CPU::CP_HL(void) { // 0xbe
  // compare A with *(HL)
  unsigned char result, operand, temp_reg_a;
  unsigned char hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  operand = mem->read_byte(registers.hl);
  hcarry_test = (temp_reg_a & 0x0F) - (operand & 0x0F);
  result = temp_reg_a - operand;

  set_zflag(result == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(operand > temp_reg_a);
}
void CPU::CP_A(void) { // 0xbf
  set_zflag(true);
  set_nflag(true);
  set_hflag(false);
  set_cflag(false);
}
void CPU::RET_NZ(void) { // 0xc0
  if (check_zflag() == false) {
    branch_taken = true;
    registers.pc = mem->read_short_stack(registers.sp);
    registers.sp += 2;
  } else {
    branch_taken = false;
    registers.pc += instrs[0xC0].operand_length;
  }
}
void CPU::POP_BC(void) { // 0xc1
  registers.bc = mem->read_short_stack(registers.sp);
  registers.sp += 2;
}
void CPU::JP_NZ_a16(void) { // 0xc2
  if (check_zflag() == false) {
    branch_taken = true;
    registers.pc = mem->read_short(registers.pc);
  } else {
    branch_taken = false;
    registers.pc += instrs[0xC2].operand_length;
  }
}
void CPU::JP_a16(void) { // 0xc3
  registers.pc = mem->read_short(registers.pc);
}
void CPU::CALL_NZ_a16(void) { // 0xc4
  // push current pc onto stack, decrement sp, and jump to imm
  if (check_zflag() == false) {
    branch_taken = true;
    unsigned short operand = mem->read_short(registers.pc);
    registers.pc += 2;
    mem->write_short_to_stack(registers.sp, registers.pc);
    registers.sp -= 2;
    registers.pc = operand;
  } else {
    branch_taken = false;
    registers.pc += instrs[0xC4].operand_length;
  }
}
void CPU::PUSH_BC(void) { // 0xc5
  mem->write_short_to_stack(registers.sp, registers.bc);
  registers.sp -= 2;
}
void CPU::ADD_A_d8(void) { // 0xc6
  unsigned short carry_test;
  unsigned char hcarry_test;
  unsigned char temp_reg_a, temp_reg;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg = mem->read_byte(registers.pc);

  hcarry_test = (temp_reg_a & 0x0F) + (temp_reg & 0x0F);
  carry_test = temp_reg_a + temp_reg;
  temp_reg_a = temp_reg_a + temp_reg;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0);
  set_nflag(false);
  set_hflag(hcarry_test & 0x10);
  set_cflag(carry_test & 0x0100);
}
void CPU::RST_00H(void) { // 0xc7
  mem->write_short_to_stack(registers.sp, registers.pc);
  registers.sp -= 2;

  registers.pc = 0x0000;
}
void CPU::RET_Z(void) { // 0xc8
  if (check_zflag() == true) {
    branch_taken = true;
    registers.pc = mem->read_short_stack(registers.sp);
    registers.sp += 2;
  } else {
    branch_taken = false;
    registers.pc += instrs[0xC8].operand_length;
  }
}
void CPU::RET(void) { // 0xc9
  registers.pc = mem->read_short_stack(registers.sp);
  registers.sp += 2;
}
void CPU::JP_Z_a16(void) { // 0xca
  if (check_zflag() == true) {
    branch_taken = true;
    registers.pc = mem->read_short(registers.pc);
  } else {
    branch_taken = false;
    registers.pc += instrs[0xCA].operand_length;
  }
}

void CPU::CB_RLC(unsigned char *reg_ptr) {
  set_cflag( !!(*reg_ptr & 0x80));
  *reg_ptr = ((*reg_ptr << 1) & 0xFE) | ((*reg_ptr >> 7) & 0x01);
  set_zflag(!!(*reg_ptr == 0x00));
  set_nflag(0);
  set_hflag(0);
}

void CPU::CB_RRC(unsigned char *reg_ptr) {
  set_cflag( !!(*reg_ptr & 0x01));
  *reg_ptr = ((*reg_ptr << 7) & 0x80) | ((*reg_ptr >> 1) & 0x7F);
  set_zflag(!!(*reg_ptr == 0x00));
  set_nflag(0);
  set_hflag(0);
}

void CPU::CB_RL(unsigned char *reg_ptr) {
  unsigned char temp_reg = *reg_ptr; // prev reg state
  *reg_ptr = ((*reg_ptr << 1) & 0xFE) | ((!!check_cflag() << 0) & 0x01);
  set_zflag(!!(*reg_ptr == 0x00));
  set_nflag(0);
  set_hflag(0);
  set_cflag( !!(temp_reg & 0x80));
}

void CPU::CB_RR(unsigned char *reg_ptr) {
  unsigned char temp_reg = *reg_ptr; // prev reg state
  *reg_ptr = ((*reg_ptr >> 1) & 0x7F) | ((!!check_cflag() << 7) & 0x80);
  set_zflag(!!(*reg_ptr == 0x00));
  set_nflag(0);
  set_hflag(0);
  set_cflag( !!(temp_reg & 0x01));
}

void CPU::CB_SLA(unsigned char *reg_ptr) {
  set_cflag( !!(*reg_ptr & 0x80));
  *reg_ptr = *reg_ptr << 1;
  set_zflag(!!(*reg_ptr == 0x00));
  set_nflag(0);
  set_hflag(0);

}

void CPU::CB_SRA(unsigned char *reg_ptr) {
  set_cflag( !!(*reg_ptr & 0x01));
  *reg_ptr = (*reg_ptr >> 1) | (*reg_ptr & 0x80); // preserve sign bit
  set_zflag(!!(*reg_ptr == 0x00));
  set_nflag(0);
  set_hflag(0);
}

void CPU::CB_SWAP(unsigned char *reg_ptr) {
  *reg_ptr = ((*reg_ptr >> 4) & 0x0F) | ((*reg_ptr << 4) & 0xF0);
  set_zflag(!!(*reg_ptr == 0x00));
  set_nflag(0);
  set_hflag(0);
  set_cflag(0);
}

void CPU::CB_SRL(unsigned char *reg_ptr) {
  set_cflag(!!(*reg_ptr & 0x01));
  *reg_ptr = (*reg_ptr >> 1); 
  set_zflag(!!(*reg_ptr == 0x00));
  set_nflag(0);
  set_hflag(0);
}

void CPU::PREFIX_CB(void) { // 0xcb
  unsigned char cb_opcode, x, y, z;
  unsigned char *reg_ptr;

  cb_opcode = mem->read_byte(registers.pc);
  x = get_x(cb_opcode);   // opcode type
  y = get_y(cb_opcode);   // operand: rotate type or bit number
  z = get_z(cb_opcode);   // operand: register

  // operand register
  reg_ptr = CB_Z_Register_Table[z];
  // read value from mem if (hl) is operand reg
  if (reg_ptr == &hl_temp_reg) {
      hl_temp_reg = mem->read_byte(registers.hl);
  }

  // instruction type
  switch (x) {
    case 0:  // rotate or shift
      (this->*(CB_Y_Rotate_Shift_Table[y]))(reg_ptr); 
      break;
    case 1:  // test bit
      set_zflag( !((1 << y) & *reg_ptr));
      set_nflag(0);
      set_hflag(1);
      break;
    case 2:  // reset bit
      *reg_ptr &= ~(1 << y);
      break;
    case 3:  // set bit
      *reg_ptr |= (1 << y);
      break;
  }

  // write value back to memory (hl)
  if (reg_ptr == &hl_temp_reg) {
    mem->write_byte(registers.hl, hl_temp_reg);
  }
}

void CPU::CALL_Z_a16(void) { // 0xcc
  // push current pc onto stack, decrement sp, and jump to imm
  if (check_zflag() == true) {
    branch_taken = true;
    unsigned short operand = mem->read_short(registers.pc);
    registers.pc += 2;
    mem->write_short_to_stack(registers.sp, registers.pc);
    registers.sp -= 2;
    registers.pc = operand;
  } else {
    branch_taken = false;
    registers.pc += instrs[0xCC].operand_length;
  }
}
void CPU::CALL_a16(void) { // 0xcd
  // push current pc onto stack, decrement sp, and jump to imm
  unsigned short operand = mem->read_short(registers.pc);

  registers.pc += 2; // this is kind of a weird one.
  mem->write_short_to_stack(registers.sp, registers.pc);
  registers.sp -= 2;
  registers.pc = operand;
}
void CPU::ADC_A_d8(void) { // 0xce
  unsigned short carry_test;
  unsigned char temp_reg_a, temp_reg, hcarry_test;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg = mem->read_byte(registers.pc);

  if (check_cflag() == true) {
    hcarry_test = (temp_reg_a & 0x0F) + (temp_reg & 0x0F) + 1;
    carry_test = temp_reg_a + temp_reg + 1;
    temp_reg_a = temp_reg_a + temp_reg + 1;
  } else {
    hcarry_test = (temp_reg_a & 0x0F) + (temp_reg & 0x0F);
    carry_test = temp_reg_a + temp_reg;
    temp_reg_a = temp_reg_a + temp_reg;
  }

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0x0000);
  set_nflag(false);
  set_hflag(hcarry_test >= 0x10);
  set_cflag(carry_test >= 0x0100);
}
void CPU::RST_08H(void) { // 0xcf
  mem->write_short_to_stack(registers.sp, registers.pc);
  registers.sp -= 2;

  registers.pc = 0x0008;
}
void CPU::RET_NC(void) { // 0xd0
  if (check_cflag() == false) {
    branch_taken = true;
    registers.pc = mem->read_short_stack(registers.sp);
    registers.sp += 2;
  } else {
    branch_taken = false;
    registers.pc += instrs[0xD0].operand_length;
  }
}
void CPU::POP_DE(void) { // 0xd1
  registers.de = mem->read_short_stack(registers.sp);
  registers.sp += 2;
}
void CPU::JP_NC_a16(void) { // 0xd2
  if (check_cflag() == false) {
    branch_taken = true;
    registers.pc = mem->read_short(registers.pc);
  } else {
    branch_taken = false;
    registers.pc += instrs[0xD2].operand_length;
  }
}
// void CPU::GARBAGE(void); //0xd3
void CPU::CALL_NC_a16(void) { // 0xd4
  // push current pc onto stack, decrement sp, and jump to imm
  if (check_cflag() == false) {
    branch_taken = true;
    unsigned short operand = mem->read_short(registers.pc);
    registers.pc += 2;
    mem->write_short_to_stack(registers.sp, registers.pc);
    registers.sp -= 2;
    registers.pc = operand;
  } else {
    branch_taken = false;
    registers.pc += instrs[0xD4].operand_length;
  }
}
void CPU::PUSH_DE(void) { // 0xd5
  mem->write_short_to_stack(registers.sp, registers.de);
  registers.sp -= 2;
}
void CPU::SUB_d8(void) { // 0xd6
  unsigned char temp_reg_a;
  unsigned char operand;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  operand = mem->read_byte(registers.pc);

  set_cflag(temp_reg_a < operand);

  temp_reg_a = temp_reg_a - operand;

  set_zflag(temp_reg_a == 0x00);
  set_nflag(true);
  set_hflag((temp_reg_a & 0x0F) > ((registers.af >> 8) & 0x0F));

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::RST_10H(void) { // 0xd7
  mem->write_short_to_stack(registers.sp, registers.pc);
  registers.sp -= 2;

  registers.pc = 0x0010;
}
void CPU::RET_C(void) { // 0xd8
  if (check_cflag() == true) {
    branch_taken = true;
    registers.pc = mem->read_short_stack(registers.sp);
    registers.sp += 2;
  } else {
    branch_taken = false;
    registers.pc += instrs[0xD8].operand_length;
  }
}
void CPU::RETI(void) { // 0xd9
  registers.pc = mem->read_short_stack(registers.sp);
  registers.sp += 2;
  interrupt->ime_flag = true;
}
void CPU::JP_C_a16(void) { // 0xda
  if (check_cflag() == true) {
    branch_taken = true;
    registers.pc = mem->read_short(registers.pc);
  } else {
    branch_taken = false;
    registers.pc += instrs[0xDA].operand_length;
  }
}
void CPU::CALL_C_a16(void) { // 0xdc
  // push current pc onto stack, decrement sp, and jump to imm
  if (check_cflag() == true) {
    branch_taken = true;
    unsigned short operand = mem->read_short(registers.pc);
    registers.pc += 2;
    mem->write_short_to_stack(registers.sp, registers.pc);
    registers.sp -= 2;
    registers.pc = operand;
  } else {
    branch_taken = false;
    registers.pc += instrs[0xC4].operand_length;
  }
}
void CPU::SBC_A_d8(void) { // 0xde
  unsigned short temp_reg_a, temp_reg;
  unsigned char hcarry_test;
  // A = A - (d8 + cflag)
  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg = mem->read_byte(registers.pc);

  hcarry_test =
      (temp_reg_a & 0x0F) - (temp_reg & 0x0F) - (check_cflag() == true);

  if (check_cflag())
    temp_reg++;

  set_cflag(temp_reg_a < temp_reg);

  temp_reg_a -= temp_reg;

  set_zflag((temp_reg_a & 0x00FF) == 0x00);
  set_nflag(true);
  set_hflag(hcarry_test >= 0x10);

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::RST_18H(void) { // 0xdf
  mem->write_short_to_stack(registers.sp, registers.pc);
  registers.sp -= 2;

  registers.pc = 0x0018;
}
void CPU::LDH_a8_A(void) { // 0xe0
  unsigned char operand;
  unsigned short offset;

  operand = ((registers.af & 0xFF00) >> 8);
  offset = 0xFF00 + mem->read_byte(registers.pc);
  mem->write_byte(offset, operand);
}
void CPU::POP_HL(void) { // 0xe1
  registers.hl = mem->read_short_stack(registers.sp);
  registers.sp += 2;
}
void CPU::LD_C_A_offs(void) { // 0xe2
  unsigned char offset, operand;

  offset = registers.bc & 0x00FF;
  operand = ((registers.af & 0xFF00) >> 8);
  mem->write_byte(0xFF00 + offset, operand);
}
void CPU::PUSH_HL(void) { // 0xe5
  mem->write_short_to_stack(registers.sp, registers.hl);
  registers.sp -= 2;
}
void CPU::AND_d8(void) { // 0xe6
  unsigned char temp_reg_a;
  unsigned char operand;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  operand = mem->read_byte(registers.pc);
  temp_reg_a &= operand;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0x00);
  set_nflag(false);
  set_hflag(true);
  set_cflag(false);
}
void CPU::RST_20H(void) { // 0xe7
  mem->write_short_to_stack(registers.sp, registers.pc);
  registers.sp -= 2;

  registers.pc = 0x0020;
}
void CPU::ADD_SP_r8(void) { // 0xe8
  unsigned char operand;
  unsigned carry_test;
  unsigned char hcarry_test;

  operand = mem->read_byte(registers.pc);
  if (operand & 0x80) {
    hcarry_test = (registers.sp & 0x000F) + (operand & 0x0F);
    carry_test = (registers.sp & 0x00FF) + (operand & 0xFF);
    operand ^= 0xFF;
    operand++;
    registers.sp -= operand;

  } else {
    hcarry_test = (registers.sp & 0x000F) + (operand & 0x0F);
    carry_test = (registers.sp & 0x00FF) + (operand & 0xFF);
    registers.sp += operand;
  }

  set_zflag(false);
  set_nflag(false);
  set_hflag(hcarry_test & 0x10);
  set_cflag(!!(carry_test >= 0x00000100));
}
void CPU::JP_HL(void) { // 0xe9
  registers.pc = registers.hl;
}
void CPU::LD_a16_A(void) { // 0xea
  // *(nn) = A
  unsigned char temp_reg;
  unsigned short addr;

  temp_reg = ((registers.af & 0xFF00) >> 8);
  addr = mem->read_short(registers.pc);
  mem->write_byte(addr, temp_reg);
}
void CPU::XOR_d8(void) { // 0xee
  unsigned char temp_reg_a, temp_reg;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  temp_reg = mem->read_byte(registers.pc);
  temp_reg_a ^= temp_reg;

  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0x00);
  set_nflag(false);
  set_hflag(false);
  set_cflag(false);
}
void CPU::RST_28H(void) { // 0xef
  mem->write_short_to_stack(registers.sp, registers.pc);
  registers.sp -= 2;

  registers.pc = 0x0028;
}
void CPU::LDH_A_a8(void) { // 0xf0
  // A = *(nn+0xff00)
  unsigned short offset;
  unsigned char temp_reg;

  offset = 0xFF00 + mem->read_byte(registers.pc);
  temp_reg = mem->read_byte(offset);
  registers.af = ((temp_reg << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::POP_AF(void) { // 0xf1
  registers.af = mem->read_short_stack(registers.sp);
  registers.af &= 0xFFF0; // last nibble of f is unused
  registers.sp += 2;
}
void CPU::LD_A_C2(void) { // 0xf2
  // A = *(C +0xff00)
  unsigned short offset;
  unsigned char temp_reg;

  offset = 0xFF00 + (registers.bc & 0x00FF);
  temp_reg = mem->read_byte(offset);
  registers.af = ((temp_reg << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::DI(void) { // 0xf3
  // disable interrupts
  interrupt->ime_flag = false;
}
void CPU::PUSH_AF(void) { // 0xf5
  mem->write_short_to_stack(registers.sp, registers.af);
  registers.sp -= 2;
}
void CPU::OR_d8(void) { // 0xf6
  unsigned char temp_reg_a, operand;

  temp_reg_a = ((registers.af >> 8) & 0x00FF);
  operand = mem->read_byte(registers.pc);
  temp_reg_a |= operand;
  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);

  set_zflag(temp_reg_a == 0x00);
  set_nflag(false);
  set_hflag(false);
  set_cflag(false);
}
void CPU::RST_30H(void) { // 0xf7
  mem->write_short_to_stack(registers.sp, registers.pc);
  registers.sp -= 2;

  registers.pc = 0x0030;
}
void CPU::LD_HL_SP_r8(void) { // 0xf8
  unsigned char operand;
  unsigned carry_test;
  unsigned char hcarry_test;

  operand = mem->read_byte(registers.pc);
  hcarry_test = (registers.sp & 0x000F) + (operand & 0x0F);
  carry_test = (operand & 0xFF) + (registers.sp & 0x00FF & 0x00FF);
  registers.hl = (signed char)operand + registers.sp;

  set_zflag(false);
  set_nflag(false);
  set_hflag(hcarry_test & 0x10);
  set_cflag(carry_test >= 0x00000100);
}
void CPU::LD_SP_HL(void) { // 0xf9
  registers.sp = registers.hl;
}
void CPU::LD_A_a16(void) { // 0xfa
  unsigned char temp_reg_a;
  unsigned short operand_addr;

  operand_addr = mem->read_short(registers.pc);
  temp_reg_a = mem->read_byte(operand_addr);
  registers.af = ((temp_reg_a << 8) & 0xFF00) | (registers.af & 0x00FF);
}
void CPU::EI(void) { // 0xfb
  interrupt->ime_flag = true;
}
void CPU::CP_d8(void) { // 0xfe
  unsigned char operand;
  unsigned char hcarry_test;
  unsigned char temp_reg, result;

  operand = mem->read_byte(registers.pc);
  temp_reg = ((registers.af >> 8) & 0x00FF);
  hcarry_test = (temp_reg & 0x0F) - (operand & 0x0F);
  result = temp_reg - operand;

  set_zflag(!!(result == 0x00));
  set_nflag(true);
  set_hflag(hcarry_test & 0x10);
  set_cflag(!!(operand > temp_reg));
}
void CPU::RST_38H(void) { // 0xff
  mem->write_short_to_stack(registers.sp, registers.pc);
  registers.sp -= 2;

  registers.pc = 0x0038;
}

void CPU::init(GB_Sys *gb_sys) {
  mem = gb_sys->mem;
  interrupt = gb_sys->interrupt;
}
