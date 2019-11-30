#pragma once
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <boost/circular_buffer.hpp>
#include "gbcon.h"

class CPU {
public:
  unsigned int ticks = 0;                               // instrs exec'd
  unsigned long int machine_cycle_counter = 0x00000000; // machine cycles

  /* the flag register
    Bit  Name  Set Clr  Expl.
    7    zf    Z   NZ   Zero Flag
    6    n     -   -    Add/Sub-Flag (BCD)
    5    h     -   -    Half Carry Flag (BCD)
    4    cy    C   NC   Carry Flag
    3-0  -     -   -    Not used (always zero)
  */

  typedef struct registers_t {
    union {
      unsigned short af; // accumulator & flags
      struct {
        unsigned char f, a;
      };
    };

    union {
      unsigned short bc;
      struct {
        unsigned char c, b;
      };
    };
    union {
      unsigned short de;
      struct {
        unsigned char e, d;
      };
    };
    union {
      unsigned short hl;
      struct {
        unsigned char l, h;
      };
    };
    unsigned short pc; // program counter
    unsigned short sp; // stack pointer
  } registers_t;

  registers_t registers;

  unsigned char get_x(unsigned char opcode);
  unsigned char get_y(unsigned char opcode);
  unsigned char get_z(unsigned char opcode);

  void set_zflag(bool val);
  void set_nflag(bool val);
  void set_hflag(bool val);
  void set_cflag(bool val);
  bool check_zflag(void);
  bool check_nflag(void);
  bool check_hflag(void);
  bool check_cflag(void);

  bool stop = false;
  bool halted = false;
  bool branch_taken = false;

  typedef void (CPU::*fptr)(void);

  struct instruction {
    std::string disassembly;
    unsigned char operand_length;
    unsigned char cycle_duration_sh;
    unsigned char cycle_duration_l;
    bool prog_control_inst;
    bool cond_control_inst;
    fptr execute;
  };

  void reset(void);
  unsigned int cpu_step(void);

  void nop(void);         // 0x00
  void LD_BC_d16(void);   // 0x01
  void LD_BC_A(void);     // 0x02
  void INC_BC(void);      // 0x03
  void INC_B(void);       // 0x04
  void DEC_B(void);       // 0x05
  void LD_B_d8(void);     // 0x06
  void RLCA(void);        // 0x07
  void LD_a16_SP(void);   // 0x08
  void ADD_HL_BC(void);   // 0x09
  void LD_A_BC(void);     // 0x0a
  void DEC_BC(void);      // 0x0b
  void INC_C(void);       // 0x0c
  void DEC_C(void);       // 0x0d
  void LD_C_d8(void);     // 0x0e
  void RRCA(void);        // 0x0f
  void STOP_0(void);      // 0x10
  void LD_DE_d16(void);   // 0x11
  void LD_DE_A(void);     // 0x12
  void INC_DE(void);      // 0x13
  void INC_D(void);       // 0x14
  void DEC_D(void);       // 0x15
  void LD_D_d8(void);     // 0x16
  void RLA(void);         // 0x17
  void JR_r8(void);       // 0x18
  void ADD_HL_DE(void);   // 0x19
  void LD_A_DE(void);     // 0x1a
  void DEC_DE(void);      // 0x1b
  void INC_E(void);       // 0x1c
  void DEC_E(void);       // 0x1d
  void LD_E_d8(void);     // 0x1e
  void RRA(void);         // 0x1f
  void JR_NZ_r8(void);    // 0x20
  void LD_HL_d16(void);   // 0x21
  void LD_HLp_A(void);    // 0x22
  void INC_HL(void);      // 0x23
  void INC_H(void);       // 0x24
  void DEC_H(void);       // 0x25
  void LD_H_d8(void);     // 0x26
  void DAA(void);         // 0x27
  void JR_Z_r8(void);     // 0x28
  void ADD_HL_HL(void);   // 0x29
  void LD_A_HL_pl(void);  // 0x2a
  void DEC_HL(void);      // 0x2b
  void INC_L(void);       // 0x2c
  void DEC_L(void);       // 0x2d
  void LD_L_d8(void);     // 0x2e
  void CPL(void);         // 0x2f
  void JR_NC_r8(void);    // 0x30
  void LD_SP_d16(void);   // 0x31
  void LD_HLm_A(void);    // 0x32
  void INC_SP(void);      // 0x33
  void INC_HL2(void);     // 0x34
  void DEC_HL2(void);     // 0x35
  void LD_HL_d8(void);    // 0x36
  void SCF(void);         // 0x37
  void JR_C_r8(void);     // 0x38
  void ADD_HL_SP(void);   // 0x39
  void LD_A_HL_min(void); // 0x3a
  void DEC_SP(void);      // 0x3b
  void INC_A(void);       // 0x3c
  void DEC_A(void);       // 0x3d
  void LD_A_d8(void);     // 0x3e
  void CCF(void);         // 0x3f
  void LD_B_B(void);      // 0x40
  void LD_B_C(void);      // 0x41
  void LD_B_D(void);      // 0x42
  void LD_B_E(void);      // 0x43
  void LD_B_H(void);      // 0x44
  void LD_B_L(void);      // 0x45
  void LD_B_HL(void);     // 0x46
  void LD_B_A(void);      // 0x47
  void LD_C_B(void);      // 0x48
  void LD_C_C(void);      // 0x49
  void LD_C_D(void);      // 0x4a
  void LD_C_E(void);      // 0x4b
  void LD_C_H(void);      // 0x4c
  void LD_C_L(void);      // 0x4d
  void LD_C_HL(void);     // 0x4e
  void LD_C_A(void);      // 0x4f
  void LD_D_B(void);      // 0x50
  void LD_D_C(void);      // 0x51
  void LD_D_D(void);      // 0x52
  void LD_D_E(void);      // 0x53
  void LD_D_H(void);      // 0x54
  void LD_D_L(void);      // 0x55
  void LD_D_HL(void);     // 0x56
  void LD_D_A(void);      // 0x57
  void LD_E_B(void);      // 0x58
  void LD_E_C(void);      // 0x59
  void LD_E_D(void);      // 0x5a
  void LD_E_E(void);      // 0x5b
  void LD_E_H(void);      // 0x5c
  void LD_E_L(void);      // 0x5d
  void LD_E_HL(void);     // 0x5e
  void LD_E_A(void);      // 0x5f
  void LD_H_B(void);      // 0x60
  void LD_H_C(void);      // 0x61
  void LD_H_D(void);      // 0x62
  void LD_H_E(void);      // 0x63
  void LD_H_H(void);      // 0x64
  void LD_H_L(void);      // 0x65
  void LD_H_HL(void);     // 0x66
  void LD_H_A(void);      // 0x67
  void LD_L_B(void);      // 0x68
  void LD_L_C(void);      // 0x69
  void LD_L_D(void);      // 0x6a
  void LD_L_E(void);      // 0x6b
  void LD_L_H(void);      // 0x6c
  void LD_L_L(void);      // 0x6d
  void LD_L_HL(void);     // 0x6e
  void LD_L_A(void);      // 0x6f
  void LD_HL_B(void);     // 0x70
  void LD_HL_C(void);     // 0x71
  void LD_HL_D(void);     // 0x72
  void LD_HL_E(void);     // 0x73
  void LD_HL_H(void);     // 0x74
  void LD_HL_L(void);     // 0x75
  void HALT(void);        // 0x76
  void LD_HL_A(void);     // 0x77
  void LD_A_B(void);      // 0x78
  void LD_A_C(void);      // 0x79
  void LD_A_D(void);      // 0x7a
  void LD_A_E(void);      // 0x7b
  void LD_A_H(void);      // 0x7c
  void LD_A_L(void);      // 0x7d
  void LD_A_HL(void);     // 0x7e
  void LD_A_A(void);      // 0x7f
  void ADD_A_B(void);     // 0x80
  void ADD_A_C(void);     // 0x81
  void ADD_A_D(void);     // 0x82
  void ADD_A_E(void);     // 0x83
  void ADD_A_H(void);     // 0x84
  void ADD_A_L(void);     // 0x85
  void ADD_A_HL(void);    // 0x86
  void ADD_A_A(void);     // 0x87
  void ADC_A_B(void);     // 0x88
  void ADC_A_C(void);     // 0x89
  void ADC_A_D(void);     // 0x8a
  void ADC_A_E(void);     // 0x8b
  void ADC_A_H(void);     // 0x8c
  void ADC_A_L(void);     // 0x8d
  void ADC_A_HL(void);    // 0x8e
  void ADC_A_A(void);     // 0x8f
  void SUB_B(void);       // 0x90
  void SUB_C(void);       // 0x91
  void SUB_D(void);       // 0x92
  void SUB_E(void);       // 0x93
  void SUB_H(void);       // 0x94
  void SUB_L(void);       // 0x95
  void SUB_HL(void);      // 0x96
  void SUB_A(void);       // 0x97
  void SBC_A_B(void);     // 0x98
  void SBC_A_C(void);     // 0x99
  void SBC_A_D(void);     // 0x9a
  void SBC_A_E(void);     // 0x9b
  void SBC_A_H(void);     // 0x9c
  void SBC_A_L(void);     // 0x9d
  void SBC_A_HL(void);    // 0x9e
  void SBC_A_A(void);     // 0x9f
  void AND_B(void);       // 0xa0
  void AND_C(void);       // 0xa1
  void AND_D(void);       // 0xa2
  void AND_E(void);       // 0xa3
  void AND_H(void);       // 0xa4
  void AND_L(void);       // 0xa5
  void AND_HL(void);      // 0xa6
  void AND_A(void);       // 0xa7
  void XOR_B(void);       // 0xa8
  void XOR_C(void);       // 0xa9
  void XOR_D(void);       // 0xaa
  void XOR_E(void);       // 0xab
  void XOR_H(void);       // 0xac
  void XOR_L(void);       // 0xad
  void XOR_HL(void);      // 0xae
  void XOR_A(void);       // 0xaf
  void OR_B(void);        // 0xb0
  void OR_C(void);        // 0xb1
  void OR_D(void);        // 0xb2
  void OR_E(void);        // 0xb3
  void OR_H(void);        // 0xb4
  void OR_L(void);        // 0xb5
  void OR_HL(void);       // 0xb6
  void OR_A(void);        // 0xb7
  void CP_B(void);        // 0xb8
  void CP_C(void);        // 0xb9
  void CP_D(void);        // 0xba
  void CP_E(void);        // 0xbb
  void CP_H(void);        // 0xbc
  void CP_L(void);        // 0xbd
  void CP_HL(void);       // 0xbe
  void CP_A(void);        // 0xbf
  void RET_NZ(void);      // 0xc0
  void POP_BC(void);      // 0xc1
  void JP_NZ_a16(void);   // 0xc2
  void JP_a16(void);      // 0xc3
  void CALL_NZ_a16(void); // 0xc4
  void PUSH_BC(void);     // 0xc5
  void ADD_A_d8(void);    // 0xc6
  void RST_00H(void);     // 0xc7
  void RET_Z(void);       // 0xc8
  void RET(void);         // 0xc9
  void JP_Z_a16(void);    // 0xca
  void PREFIX_CB(void);   // 0xcb
  void CALL_Z_a16(void);  // 0xcc
  void CALL_a16(void);    // 0xcd
  void ADC_A_d8(void);    // 0xce
  void RST_08H(void);     // 0xcf
  void RET_NC(void);      // 0xd0
  void POP_DE(void);      // 0xd1
  void JP_NC_a16(void);   // 0xd2
  void GARBAGE(void);     // 0xd3
  void CALL_NC_a16(void); // 0xd4
  void PUSH_DE(void);     // 0xd5
  void SUB_d8(void);      // 0xd6
  void RST_10H(void);     // 0xd7
  void RET_C(void);       // 0xd8
  void RETI(void);        // 0xd9
  void JP_C_a16(void);    // 0xda
  void CALL_C_a16(void);  // 0xdc
  void SBC_A_d8(void);    // 0xde
  void RST_18H(void);     // 0xdf
  void LDH_a8_A(void);    // 0xe0
  void POP_HL(void);      // 0xe1
  void LD_C_A_offs(void); // 0xe2
  void PUSH_HL(void);     // 0xe5
  void AND_d8(void);      // 0xe6
  void RST_20H(void);     // 0xe7
  void ADD_SP_r8(void);   // 0xe8
  void JP_HL(void);       // 0xe9
  void LD_a16_A(void);    // 0xea
  void XOR_d8(void);      // 0xee
  void RST_28H(void);     // 0xef
  void LDH_A_a8(void);    // 0xf0
  void POP_AF(void);      // 0xf1
  void LD_A_C2(void);     // 0xf2
  void DI(void);          // 0xf3
  void PUSH_AF(void);     // 0xf5
  void OR_d8(void);       // 0xf6
  void RST_30H(void);     // 0xf7
  void LD_HL_SP_r8(void); // 0xf8
  void LD_SP_HL(void);    // 0xf9
  void LD_A_a16(void);    // 0xfa
  void EI(void);          // 0xfb
  void CP_d8(void);       // 0xfe
  void RST_38H(void);     // 0xff

  void _unimplemented(void); //

  const struct instruction instrs[256] = {
  /* INST-LEN BYTES--CYC DUR SHORT--CYC DUR LONG--PC INST--COND INST--INST HANDLER*/
      {"NOP", 0, 4, 0, 0, 0, &CPU::nop},                   // 0x00
      {"LD BC,d16", 2, 12, 0, 0, 0, &CPU::LD_BC_d16},      // 0x01
      {"LD (BC),A", 0, 8, 0, 0, 0, &CPU::LD_BC_A},         // 0x02
      {"INC BC", 0, 8, 0, 0, 0, &CPU::INC_BC},             // 0x03
      {"INC B", 0, 4, 0, 0, 0, &CPU::INC_B},               // 0x04
      {"DEC B", 0, 4, 0, 0, 0, &CPU::DEC_B},               // 0x05
      {"LD B,d8", 1, 8, 0, 0, 0, &CPU::LD_B_d8},           // 0x06
      {"RLCA", 0, 4, 0, 0, 0, &CPU::RLCA},                 // 0x07
      {"LD (a16),SP", 2, 20, 0, 0, 0, &CPU::LD_a16_SP},    // 0x08
      {"ADD HL,BC", 0, 8, 0, 0, 0, &CPU::ADD_HL_BC},       // 0x09
      {"LD A,(BC)", 0, 8, 0, 0, 0, &CPU::LD_A_BC},         // 0x0a
      {"DEC BC", 0, 8, 0, 0, 0, &CPU::DEC_BC},             // 0x0b
      {"INC C", 0, 4, 0, 0, 0, &CPU::INC_C},               // 0x0c
      {"DEC C", 0, 4, 0, 0, 0, &CPU::DEC_C},               // 0x0d
      {"LD C,d8", 1, 8, 0, 0, 0, &CPU::LD_C_d8},           // 0x0e
      {"RRCA", 0, 4, 0, 0, 0, &CPU::RRCA},                 // 0x0f
      {"STOP 0", 1, 4, 0, 0, 0, &CPU::STOP_0},             // 0x10
      {"LD DE,d16", 2, 12, 0, 0, 0, &CPU::LD_DE_d16},      // 0x11
      {"LD (DE),A", 0, 8, 0, 0, 0, &CPU::LD_DE_A},         // 0x12
      {"INC DE", 0, 8, 0, 0, 0, &CPU::INC_DE},             // 0x13
      {"INC D", 0, 4, 0, 0, 0, &CPU::INC_D},               // 0x14
      {"DEC D", 0, 4, 0, 0, 0, &CPU::DEC_D},               // 0x15
      {"LD D,d8", 1, 8, 0, 0, 0, &CPU::LD_D_d8},           // 0x16
      {"RLA", 0, 4, 0, 0, 0, &CPU::RLA},                   // 0x17
      {"JR r8", 1, 12, 0, 1, 0, &CPU::JR_r8},              // 0x18
      {"ADD HL,DE", 0, 8, 0, 0, 0, &CPU::ADD_HL_DE},       // 0x19
      {"LD A,(DE)", 0, 8, 0, 0, 0, &CPU::LD_A_DE},         // 0x1a
      {"DEC DE", 0, 8, 0, 0, 0, &CPU::DEC_DE},             // 0x1b
      {"INC E", 0, 4, 0, 0, 0, &CPU::INC_E},               // 0x1c
      {"DEC E", 0, 4, 0, 0, 0, &CPU::DEC_E},               // 0x1d
      {"LD E,d8", 1, 8, 0, 0, 0, &CPU::LD_E_d8},           // 0x1e
      {"RRA", 0, 4, 0, 0, 0, &CPU::RRA},                   // 0x1f
      {"JR NZ,r8", 1, 8, 12, 1, 1, &CPU::JR_NZ_r8},        // 0x20
      {"LD HL,d16", 2, 12, 0, 0, 0, &CPU::LD_HL_d16},      // 0x21
      {"LD (HL+),A", 0, 8, 0, 0, 0, &CPU::LD_HLp_A},       // 0x22
      {"INC HL", 0, 8, 0, 0, 0, &CPU::INC_HL},             // 0x23
      {"INC H", 0, 4, 0, 0, 0, &CPU::INC_H},               // 0x24
      {"DEC H", 0, 4, 0, 0, 0, &CPU::DEC_H},               // 0x25
      {"LD H,d8", 1, 8, 0, 0, 0, &CPU::LD_H_d8},           // 0x26
      {"DAA", 0, 4, 0, 0, 0, &CPU::DAA},                   // 0x27
      {"JR Z,r8", 1, 8, 12, 1, 1, &CPU::JR_Z_r8},          // 0x28
      {"ADD HL,HL", 0, 8, 0, 0, 0, &CPU::ADD_HL_HL},       // 0x29
      {"LD A,(HL+)", 0, 8, 0, 0, 0, &CPU::LD_A_HL_pl},     // 0x2a
      {"DEC HL", 0, 8, 0, 0, 0, &CPU::DEC_HL},             // 0x2b
      {"INC L", 0, 4, 0, 0, 0, &CPU::INC_L},               // 0x2c
      {"DEC L", 0, 4, 0, 0, 0, &CPU::DEC_L},               // 0x2d
      {"LD L,d8", 1, 8, 0, 0, 0, &CPU::LD_L_d8},           // 0x2e
      {"CPL", 0, 4, 0, 0, 0, &CPU::CPL},                   // 0x2f
      {"JR NC,r8", 1, 8, 12, 1, 0, &CPU::JR_NC_r8},        // 0x30
      {"LD SP,d16", 2, 12, 0, 0, 0, &CPU::LD_SP_d16},      // 0x31
      {"LD (HL-),A", 0, 8, 0, 0, 0, &CPU::LD_HLm_A},       // 0x32
      {"INC SP", 0, 8, 0, 0, 0, &CPU::INC_SP},             // 0x33
      {"INC (HL)", 0, 12, 0, 0, 0, &CPU::INC_HL2},         // 0x34
      {"DEC (HL)", 0, 12, 0, 0, 0, &CPU::DEC_HL2},         // 0x35
      {"LD (HL),d8", 1, 12, 0, 0, 0, &CPU::LD_HL_d8},      // 0x36
      {"SCF", 0, 4, 0, 0, 0, &CPU::SCF},                   // 0x37
      {"JR C,r8", 1, 8, 12, 1, 1, &CPU::JR_C_r8},          // 0x38
      {"ADD HL,SP", 0, 8, 0, 0, 0, &CPU::ADD_HL_SP},       // 0x39
      {"LD A,(HL-)", 0, 8, 0, 0, 0, &CPU::LD_A_HL_min},    // 0x3a
      {"DEC SP", 0, 8, 0, 0, 0, &CPU::DEC_SP},             // 0x3b
      {"INC A", 0, 4, 0, 0, 0, &CPU::INC_A},               // 0x3c
      {"DEC A", 0, 4, 0, 0, 0, &CPU::DEC_A},               // 0x3d
      {"LD A,d8", 1, 8, 0, 0, 0, &CPU::LD_A_d8},           // 0x3e
      {"CCF", 0, 4, 0, 0, 0, &CPU::CCF},                   // 0x3f
      {"LD B,B", 0, 4, 0, 0, 0, &CPU::LD_B_B},             // 0x40
      {"LD B,C", 0, 4, 0, 0, 0, &CPU::LD_B_C},             // 0x41
      {"LD B,D", 0, 4, 0, 0, 0, &CPU::LD_B_D},             // 0x42
      {"LD B,E", 0, 4, 0, 0, 0, &CPU::LD_B_E},             // 0x43
      {"LD B,H", 0, 4, 0, 0, 0, &CPU::LD_B_H},             // 0x44
      {"LD B,L", 0, 4, 0, 0, 0, &CPU::LD_B_L},             // 0x45
      {"LD B,(HL)", 0, 8, 0, 0, 0, &CPU::LD_B_HL},         // 0x46
      {"LD B,A", 0, 4, 0, 0, 0, &CPU::LD_B_A},             // 0x47
      {"LD C,B", 0, 4, 0, 0, 0, &CPU::LD_C_B},             // 0x48
      {"LD C,C", 0, 4, 0, 0, 0, &CPU::LD_C_C},             // 0x49
      {"LD C,D", 0, 4, 0, 0, 0, &CPU::LD_C_D},             // 0x4a
      {"LD C,E", 0, 4, 0, 0, 0, &CPU::LD_C_E},             // 0x4b
      {"LD C,H", 0, 4, 0, 0, 0, &CPU::LD_C_H},             // 0x4c
      {"LD C,L", 0, 4, 0, 0, 0, &CPU::LD_C_L},             // 0x4d
      {"LD C,(HL)", 0, 8, 0, 0, 0, &CPU::LD_C_HL},         // 0x4e
      {"LD C,A", 0, 4, 0, 0, 0, &CPU::LD_C_A},             // 0x4f
      {"LD D,B", 0, 4, 0, 0, 0, &CPU::LD_D_B},             // 0x50
      {"LD D,C", 0, 4, 0, 0, 0, &CPU::LD_D_C},             // 0x51
      {"LD D,D", 0, 4, 0, 0, 0, &CPU::LD_D_D},             // 0x52
      {"LD D,E", 0, 4, 0, 0, 0, &CPU::LD_D_E},             // 0x53
      {"LD D,H", 0, 4, 0, 0, 0, &CPU::LD_D_H},             // 0x54
      {"LD D,L", 0, 4, 0, 0, 0, &CPU::LD_D_L},             // 0x55
      {"LD D,(HL)", 0, 8, 0, 0, 0, &CPU::LD_D_HL},         // 0x56
      {"LD D,A", 0, 4, 0, 0, 0, &CPU::LD_D_A},             // 0x57
      {"LD E,B", 0, 4, 0, 0, 0, &CPU::LD_E_B},             // 0x58
      {"LD E,C", 0, 4, 0, 0, 0, &CPU::LD_E_C},             // 0x59
      {"LD E,D", 0, 4, 0, 0, 0, &CPU::LD_E_D},             // 0x5a
      {"LD E,E", 0, 4, 0, 0, 0, &CPU::LD_E_E},             // 0x5b
      {"LD E,H", 0, 4, 0, 0, 0, &CPU::LD_E_H},             // 0x5c
      {"LD E,L", 0, 4, 0, 0, 0, &CPU::LD_E_L},             // 0x5d
      {"LD E,(HL)", 0, 8, 0, 0, 0, &CPU::LD_E_HL},         // 0x5e
      {"LD E,A", 0, 4, 0, 0, 0, &CPU::LD_E_A},             // 0x5f
      {"LD H,B", 0, 4, 0, 0, 0, &CPU::LD_H_B},             // 0x60
      {"LD H,C", 0, 4, 0, 0, 0, &CPU::LD_H_C},             // 0x61
      {"LD H,D", 0, 4, 0, 0, 0, &CPU::LD_H_D},             // 0x62
      {"LD H,E", 0, 4, 0, 0, 0, &CPU::LD_H_E},             // 0x63
      {"LD H,H", 0, 4, 0, 0, 0, &CPU::LD_H_H},             // 0x64
      {"LD H,L", 0, 4, 0, 0, 0, &CPU::LD_H_L},             // 0x65
      {"LD H,(HL)", 0, 8, 0, 0, 0, &CPU::LD_H_HL},         // 0x66
      {"LD H,A", 0, 4, 0, 0, 0, &CPU::LD_H_A},             // 0x67
      {"LD L,B", 0, 4, 0, 0, 0, &CPU::LD_L_B},             // 0x68
      {"LD L,C", 0, 4, 0, 0, 0, &CPU::LD_L_C},             // 0x69
      {"LD L,D", 0, 4, 0, 0, 0, &CPU::LD_L_D},             // 0x6a
      {"LD L,E", 0, 4, 0, 0, 0, &CPU::LD_L_E},             // 0x6b
      {"LD L,H", 0, 4, 0, 0, 0, &CPU::LD_L_H},             // 0x6c
      {"LD L,L", 0, 4, 0, 0, 0, &CPU::LD_L_L},             // 0x6d
      {"LD L,(HL)", 0, 8, 0, 0, 0, &CPU::LD_L_HL},         // 0x6e
      {"LD L,A", 0, 4, 0, 0, 0, &CPU::LD_L_A},             // 0x6f
      {"LD (HL),B", 0, 8, 0, 0, 0, &CPU::LD_HL_B},         // 0x70
      {"LD (HL),C", 0, 8, 0, 0, 0, &CPU::LD_HL_C},         // 0x71
      {"LD (HL),D", 0, 8, 0, 0, 0, &CPU::LD_HL_D},         // 0x72
      {"LD (HL),E", 0, 8, 0, 0, 0, &CPU::LD_HL_E},         // 0x73
      {"LD (HL),H", 0, 8, 0, 0, 0, &CPU::LD_HL_H},         // 0x74
      {"LD (HL),L", 0, 8, 0, 0, 0, &CPU::LD_HL_L},         // 0x75
      {"HALT", 0, 4, 0, 0, 0, &CPU::HALT},                 // 0x76
      {"LD (HL),A", 0, 8, 0, 0, 0, &CPU::LD_HL_A},         // 0x77
      {"LD A,B", 0, 4, 0, 0, 0, &CPU::LD_A_B},             // 0x78
      {"LD A,C", 0, 4, 0, 0, 0, &CPU::LD_A_C},             // 0x79
      {"LD A,D", 0, 4, 0, 0, 0, &CPU::LD_A_D},             // 0x7a
      {"LD A,E", 0, 4, 0, 0, 0, &CPU::LD_A_E},             // 0x7b
      {"LD A,H", 0, 4, 0, 0, 0, &CPU::LD_A_H},             // 0x7c
      {"LD A,L", 0, 4, 0, 0, 0, &CPU::LD_A_L},             // 0x7d
      {"LD A,(HL)", 0, 8, 0, 0, 0, &CPU::LD_A_HL},         // 0x7e
      {"LD A,A", 0, 4, 0, 0, 0, &CPU::LD_A_A},             // 0x7f
      {"ADD A,B", 0, 4, 0, 0, 0, &CPU::ADD_A_B},           // 0x80
      {"ADD A,C", 0, 4, 0, 0, 0, &CPU::ADD_A_C},           // 0x81
      {"ADD A,D", 0, 4, 0, 0, 0, &CPU::ADD_A_D},           // 0x82
      {"ADD A,E", 0, 4, 0, 0, 0, &CPU::ADD_A_E},           // 0x83
      {"ADD A,H", 0, 4, 0, 0, 0, &CPU::ADD_A_H},           // 0x84
      {"ADD A,L", 0, 4, 0, 0, 0, &CPU::ADD_A_L},           // 0x85
      {"ADD A,(HL)", 0, 8, 0, 0, 0, &CPU::ADD_A_HL},       // 0x86
      {"ADD A,A", 0, 4, 0, 0, 0, &CPU::ADD_A_A},           // 0x87
      {"ADC A,B", 0, 4, 0, 0, 0, &CPU::ADC_A_B},           // 0x88
      {"ADC A,C", 0, 4, 0, 0, 0, &CPU::ADC_A_C},           // 0x89
      {"ADC A,D", 0, 4, 0, 0, 0, &CPU::ADC_A_D},           // 0x8a
      {"ADC A,E", 0, 4, 0, 0, 0, &CPU::ADC_A_E},           // 0x8b
      {"ADC A,H", 0, 4, 0, 0, 0, &CPU::ADC_A_H},           // 0x8c
      {"ADC A,L", 0, 4, 0, 0, 0, &CPU::ADC_A_L},           // 0x8d
      {"ADC A,(HL)", 0, 8, 0, 0, 0, &CPU::ADC_A_HL},       // 0x8e
      {"ADC A,A", 0, 4, 0, 0, 0, &CPU::ADC_A_A},           // 0x8f
      {"SUB B", 0, 4, 0, 0, 0, &CPU::SUB_B},               // 0x90
      {"SUB C", 0, 4, 0, 0, 0, &CPU::SUB_C},               // 0x91
      {"SUB D", 0, 4, 0, 0, 0, &CPU::SUB_D},               // 0x92
      {"SUB E", 0, 4, 0, 0, 0, &CPU::SUB_E},               // 0x93
      {"SUB H", 0, 4, 0, 0, 0, &CPU::SUB_H},               // 0x94
      {"SUB L", 0, 4, 0, 0, 0, &CPU::SUB_L},               // 0x95
      {"SUB (HL)", 0, 8, 0, 0, 0, &CPU::SUB_HL},           // 0x96
      {"SUB A", 0, 4, 0, 0, 0, &CPU::SUB_A},               // 0x97
      {"SBC A,B", 0, 4, 0, 0, 0, &CPU::SBC_A_B},           // 0x98
      {"SBC A,C", 0, 4, 0, 0, 0, &CPU::SBC_A_C},           // 0x99
      {"SBC A,D", 0, 4, 0, 0, 0, &CPU::SBC_A_D},           // 0x9a
      {"SBC A,E", 0, 4, 0, 0, 0, &CPU::SBC_A_E},           // 0x9b
      {"SBC A,H", 0, 4, 0, 0, 0, &CPU::SBC_A_H},           // 0x9c
      {"SBC A,L", 0, 4, 0, 0, 0, &CPU::SBC_A_L},           // 0x9d
      {"SBC A,(HL)", 0, 8, 0, 0, 0, &CPU::SBC_A_HL},       // 0x9e
      {"SBC A,A", 0, 4, 0, 0, 0, &CPU::SBC_A_A},           // 0x9f
      {"AND B", 0, 4, 0, 0, 0, &CPU::AND_B},               // 0xa0
      {"AND C", 0, 4, 0, 0, 0, &CPU::AND_C},               // 0xa1
      {"AND D", 0, 4, 0, 0, 0, &CPU::AND_D},               // 0xa2
      {"AND E", 0, 4, 0, 0, 0, &CPU::AND_E},               // 0xa3
      {"AND H", 0, 4, 0, 0, 0, &CPU::AND_H},               // 0xa4
      {"AND L", 0, 4, 0, 0, 0, &CPU::AND_L},               // 0xa5
      {"AND (HL)", 0, 8, 0, 0, 0, &CPU::AND_HL},           // 0xa6
      {"AND A", 0, 4, 0, 0, 0, &CPU::AND_A},               // 0xa7
      {"XOR B", 0, 4, 0, 0, 0, &CPU::XOR_B},               // 0xa8
      {"XOR C", 0, 4, 0, 0, 0, &CPU::XOR_C},               // 0xa9
      {"XOR D", 0, 4, 0, 0, 0, &CPU::XOR_D},               // 0xaa
      {"XOR E", 0, 4, 0, 0, 0, &CPU::XOR_E},               // 0xab
      {"XOR H", 0, 4, 0, 0, 0, &CPU::XOR_H},               // 0xac
      {"XOR L", 0, 4, 0, 0, 0, &CPU::XOR_L},               // 0xad
      {"XOR (HL)", 0, 8, 0, 0, 0, &CPU::XOR_HL},           // 0xae
      {"XOR A", 0, 4, 0, 0, 0, &CPU::XOR_A},               // 0xaf
      {"OR B", 0, 4, 0, 0, 0, &CPU::OR_B},                 // 0xb0
      {"OR C", 0, 4, 0, 0, 0, &CPU::OR_C},                 // 0xb1
      {"OR D", 0, 4, 0, 0, 0, &CPU::OR_D},                 // 0xb2
      {"OR E", 0, 4, 0, 0, 0, &CPU::OR_E},                 // 0xb3
      {"OR H", 0, 4, 0, 0, 0, &CPU::OR_H},                 // 0xb4
      {"OR L", 0, 4, 0, 0, 0, &CPU::OR_L},                 // 0xb5
      {"OR (HL)", 0, 8, 0, 0, 0, &CPU::OR_HL},             // 0xb6
      {"OR A", 0, 4, 0, 0, 0, &CPU::OR_A},                 // 0xb7
      {"CP B", 0, 4, 0, 0, 0, &CPU::CP_B},                 // 0xb8
      {"CP C", 0, 4, 0, 0, 0, &CPU::CP_C},                 // 0xb9
      {"CP D", 0, 4, 0, 0, 0, &CPU::CP_D},                 // 0xba
      {"CP E", 0, 4, 0, 0, 0, &CPU::CP_E},                 // 0xbb
      {"CP H", 0, 4, 0, 0, 0, &CPU::CP_H},                 // 0xbc
      {"CP L", 0, 4, 0, 0, 0, &CPU::CP_L},                 // 0xbd
      {"CP (HL)", 0, 8, 0, 0, 0, &CPU::CP_HL},             // 0xbe
      {"CP A", 0, 4, 0, 0, 0, &CPU::CP_A},                 // 0xbf
      {"RET NZ", 0, 8, 20, 1, 1, &CPU::RET_NZ},            // 0xc0
      {"POP BC", 0, 12, 0, 0, 0, &CPU::POP_BC},            // 0xc1
      {"JP NZ,a16", 2, 12, 16, 1, 1, &CPU::JP_NZ_a16},     // 0xc2
      {"JP a16", 2, 16, 0, 1, 0, &CPU::JP_a16},            // 0xc3
      {"CALL NZ,a16", 2, 12, 24, 1, 1, &CPU::CALL_NZ_a16}, // 0xc4
      {"PUSH BC", 0, 16, 0, 0, 0, &CPU::PUSH_BC},          // 0xc5
      {"ADD A,d8", 1, 8, 0, 0, 0, &CPU::ADD_A_d8},         // 0xc6
      {"RST 00H", 0, 16, 0, 1, 0, &CPU::RST_00H},          // 0xc7
      {"RET Z", 0, 8, 20, 1, 1, &CPU::RET_Z},              // 0xc8
      {"RET", 0, 16, 0, 1, 0, &CPU::RET},                  // 0xc9
      {"JP Z,a16", 2, 12, 16, 1, 1, &CPU::JP_Z_a16},       // 0xca
      {"PREFIX CB", 0, 4, 0, 0, 0, &CPU::PREFIX_CB},       // 0xcb
      {"CALL Z,a16", 2, 12, 24, 1, 1, &CPU::CALL_Z_a16},   // 0xcc
      {"CALL a16", 2, 24, 0, 1, 0, &CPU::CALL_a16},        // 0xcd
      {"ADC A,d8", 1, 8, 0, 0, 0, &CPU::ADC_A_d8},         // 0xce
      {"RST 08H", 0, 16, 0, 1, 0, &CPU::RST_08H},          // 0xcf
      {"RET NC", 0, 8, 20, 1, 1, &CPU::RET_NC},            // 0xd0
      {"POP DE", 0, 12, 0, 0, 0, &CPU::POP_DE},            // 0xd1
      {"JP NC,a16", 2, 12, 16, 1, 1, &CPU::JP_NC_a16},     // 0xd2
      {"GARBAGE", 0, 0, 0, 0, 0, &CPU::_unimplemented},    // 0xd3
      {"CALL NC,a16", 2, 12, 24, 1, 1, &CPU::CALL_NC_a16}, // 0xd4
      {"PUSH DE", 0, 16, 0, 0, 0, &CPU::PUSH_DE},          // 0xd5
      {"SUB d8", 1, 8, 0, 0, 0, &CPU::SUB_d8},             // 0xd6
      {"RST 10H", 0, 16, 0, 1, 0, &CPU::RST_10H},          // 0xd7
      {"RET C", 0, 8, 20, 1, 1, &CPU::RET_C},              // 0xd8
      {"RETI", 0, 16, 0, 1, 0, &CPU::RETI},                // 0xd9
      {"JP C,a16", 2, 12, 16, 1, 1, &CPU::JP_C_a16},       // 0xda
      {"GARBAGE", 0, 0, 0, 0, 0, &CPU::_unimplemented},    // 0xdb
      {"CALL C,a16", 2, 12, 24, 1, 1, &CPU::CALL_C_a16},   // 0xdc
      {"GARBAGE", 0, 0, 0, 0, 0, &CPU::_unimplemented},    // 0xdd
      {"SBC A,d8", 1, 8, 0, 0, 0, &CPU::SBC_A_d8},         // 0xde
      {"RST 18H", 0, 16, 0, 1, 0, &CPU::RST_18H},          // 0xdf
      {"LDH (a8),A", 1, 12, 0, 0, 0, &CPU::LDH_a8_A},      // 0xe0
      {"POP HL", 0, 12, 0, 0, 0, &CPU::POP_HL},            // 0xe1
      {"LD (C),A", 0, 8, 0, 0, 0, &CPU::LD_C_A_offs},      // 0xe2
      {"GARBAGE", 0, 0, 0, 0, 0, &CPU::_unimplemented},    // 0xe3
      {"GARBAGE", 0, 0, 0, 0, 0, &CPU::_unimplemented},    // 0xe4
      {"PUSH HL", 0, 16, 0, 0, 0, &CPU::PUSH_HL},          // 0xe5
      {"AND d8", 1, 8, 0, 0, 0, &CPU::AND_d8},             // 0xe6
      {"RST 20H", 0, 16, 0, 1, 0, &CPU::RST_20H},          // 0xe7
      {"ADD SP,r8", 1, 16, 0, 0, 0, &CPU::ADD_SP_r8},      // 0xe8
      {"JP (HL)", 0, 4, 0, 1, 0, &CPU::JP_HL},             // 0xe9
      {"LD (a16),A", 2, 16, 0, 0, 0, &CPU::LD_a16_A},      // 0xea
      {"GARBAGE", 0, 0, 0, 0, 0, &CPU::_unimplemented},    // 0xeb
      {"GARBAGE", 0, 0, 0, 0, 0, &CPU::_unimplemented},    // 0xec
      {"GARBAGE", 0, 0, 0, 0, 0, &CPU::_unimplemented},    // 0xed
      {"XOR d8", 1, 8, 0, 0, 0, &CPU::XOR_d8},             // 0xee
      {"RST 28H", 0, 16, 0, 1, 0, &CPU::RST_28H},          // 0xef
      {"LDH A,(a8)", 1, 12, 0, 0, 0, &CPU::LDH_A_a8},      // 0xf0
      {"POP AF", 0, 12, 0, 0, 0, &CPU::POP_AF},            // 0xf1
      {"LD A,(C)", 1, 8, 0, 0, 0, &CPU::LD_A_C2},          // 0xf2
      {"DI", 0, 4, 0, 0, 0, &CPU::DI},                     // 0xf3
      {"GARBAGE", 0, 0, 0, 0, 0, &CPU::_unimplemented},    // 0xf4
      {"PUSH AF", 0, 16, 0, 0, 0, &CPU::PUSH_AF},          // 0xf5
      {"OR d8", 1, 8, 0, 0, 0, &CPU::OR_d8},               // 0xf6
      {"RST 30H", 0, 16, 0, 1, 0, &CPU::RST_30H},          // 0xf7
      {"LD HL,SP+r8", 1, 12, 0, 0, 0, &CPU::LD_HL_SP_r8},  // 0xf8
      {"LD SP,HL", 0, 8, 0, 0, 0, &CPU::LD_SP_HL},         // 0xf9
      {"LD A,(a16)", 2, 16, 0, 0, 0, &CPU::LD_A_a16},      // 0xfa
      {"EI", 0, 4, 0, 0, 0, &CPU::EI},                     // 0xfb
      {"GARBAGE", 0, 0, 0, 0, 0, &CPU::_unimplemented},    // 0xfc
      {"GARBAGE", 0, 0, 0, 0, 0, &CPU::_unimplemented},    // 0xfd
      {"CP d8", 1, 8, 0, 0, 0, &CPU::CP_d8},               // 0xfe
      {"RST 38H", 0, 16, 0, 1, 0, &CPU::RST_38H},          // 0xff
  };

  /* 0xCB Opcode Decoding Tables
   */

  void CB_RLC(unsigned char *reg_ptr);  // 0xCB 0x00 - 0xCB 0x07
  void CB_RRC(unsigned char *reg_ptr);  // 0xCB 0x08 - 0xCB 0x0f
  void CB_RL(unsigned char *reg_ptr);   // 0xCB 0x10 - 0xCB 0x07
  void CB_RR(unsigned char *reg_ptr);   // 0xCB 0x18 - 0xCB 0x1f
  void CB_SLA(unsigned char *reg_ptr);  // 0xCB 0x20 - 0xCB 0x27
  void CB_SRA(unsigned char *reg_ptr);  // 0xCB 0x28 - 0xCB 0x2f
  void CB_SWAP(unsigned char *reg_ptr); // 0xCB 0x30 - 0xCB 0x37
  void CB_SRL(unsigned char *reg_ptr);  // 0xCB 0x38 - 0xCB 0x3f

  // cb opcode function ptrs
  typedef void (CPU::*cb_fptr)(unsigned char *);

  const cb_fptr CB_Y_Rotate_Shift_Table[8] = {
      &CPU::CB_RLC,  // 0
      &CPU::CB_RRC,  // 1
      &CPU::CB_RL,   // 2
      &CPU::CB_RR,   // 3
      &CPU::CB_SLA,  // 4
      &CPU::CB_SRA,  // 5
      &CPU::CB_SWAP, // 6
      &CPU::CB_SRL,  // 7
  };

  unsigned char hl_temp_reg;
  unsigned char *CB_Z_Register_Table[8] = {
      &registers.b, // 0
      &registers.c, // 1
      &registers.d, // 2
      &registers.e, // 3
      &registers.h, // 4
      &registers.l, // 5
      &hl_temp_reg, // 6
      &registers.a, // 7
  };

  void init(GB_Sys *gb_sys);

  unsigned char curr_inst;
  unsigned short prev_pc;


private:
  /* GB system (pointers to other components)
   */

  Memory *mem;
  Interrupt *interrupt;

};
