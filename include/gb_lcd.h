#pragma once
#include <iostream>
#include "gbcon.h"

class LCD {
public:
  /* LCD register getters / setters
   */

  unsigned char get_lcdc();
  unsigned char get_stat();
  unsigned char get_scy();
  unsigned char get_scx();
  unsigned char get_ly();
  unsigned char get_lyc();
  unsigned char get_bgp();
  unsigned char get_obp0();
  unsigned char get_obp1();
  unsigned char get_wy();
  unsigned char get_wx();

  void  set_lcdc(unsigned char value);
  void  set_stat(unsigned char value);
  void  set_scy(unsigned char value);
  void  set_scx(unsigned char value);
  void  set_ly(unsigned char value);
  void  set_lyc(unsigned char value);
  void  set_bgp(unsigned char value);
  void  set_obp0(unsigned char value);
  void  set_obp1(unsigned char value);
  void  set_wy(unsigned char value);
  void  set_wx(unsigned char value);

  /* Constants
   */

  static const unsigned int LCD_Width  = 160;
  static const unsigned int LCD_Height = 144;
  const double refresh_rate_hz = 59.7;
  static const unsigned int Num_Sprites = 40;
  static const unsigned int Num_Sprites_Per_Line = 10;
  double Speed_Multi = 1.0;

  /* LCD driver and helpers
   */

  bool step(unsigned int cycles);
  void init(GB_Sys *gb_sys);

  unsigned int cycles_this_frame = 0;

private:
  /* GB system (pointers to other components)
   */

  Memory *mem;
  Interrupt *interrupt;

  /* Hardware registers
   */

  // 0xFF40 - LCD Control Register
  struct LCDC {
    bool bg_window_enabled; // bit 0
    bool sprites_enabled;
    bool sprite_size;
    bool bg_tilemap;
    bool bg_window_tileset;
    bool window_enable;
    bool window_tile_map;
    bool power; // bit 7
  } control;
  // 0xFF41 - LCD Status Register
  enum Screen_Mode {
    HBLANK = 0,
    VBLANK = 1,
    SEARCH_OAM = 2,
    DATA_TRANSFER = 3
  };
  struct STAT {
    Screen_Mode mode; // bits 1-0
    bool y_compare;
    bool mode0_enable;
    bool mode1_enable;
    bool mode2_enable;
    bool y_compare_enable; // bit 6
    // bit 7 is unused and always returns 0
  } status;
  // 0xFF42 - BG Scroll Y
  unsigned char scy;
  // 0xFF43 - BG Scroll X
  unsigned char scx;
  // 0xFF44 - LY - LCD Current Scanline
  unsigned char ly;
  // 0xFF45 - LYC - LY Compare
  unsigned char lyc;
  // 0xFF46 - DMA Control Register (handled in gb_mem.cpp)
  // 0xFF47 - BG Palette Data
  struct BGP {
    unsigned char dot_palette[4];
  } bgp;

  struct OBP {
    unsigned char dot_palette[4];
  };
  // 0xFF48 - OBP0 - OBJ Palette Data 0
  OBP obp0;
  // 0xFF49 - OBP1 - OBJ Palette Data 1
  OBP obp1;
  // 0xFF4A - WY - Window Y-Coordinate
  unsigned char wy;
  // 0xFF4B - WX - Window X-Coordinate
  unsigned char wx;

  /* LCD driver and helpers
   */

  unsigned char gb_pixels[LCD_Width * LCD_Height];
  inline void set_pixel(unsigned char x, unsigned char y, unsigned char gb_color_num);
  inline unsigned char get_pixel(unsigned char x, unsigned char y);

  void search_sprites(unsigned char line);
  void draw_sprites(unsigned char line);
  void draw_window(unsigned char line);
  void draw_background(unsigned char line);
  void render_scanline(unsigned char line);

  /* misc
   */

  Screen_Mode prev_mode;
  unsigned int ms_last_vblank = 0;

  struct Sprite_Attr_Masks {
    const unsigned char Priority = 0x80;  // bit7
    const unsigned char Y_Flip   = 0x40;
    const unsigned char X_Flip   = 0x20;
    const unsigned char Palette  = 0x10;
    const unsigned char Unused   = 0x0f;  //bit0 (unused for DMG)
  } Sprite_Attr_Masks;


  // emulator sprite metadata 
  struct Sprite_Emu {
    bool in_line_range;
    bool in_screen_x;
    unsigned short oram_addr;
  }; 
  struct Sprite {
    int y_pos;
    int x_pos;
    unsigned char tile_num;
    unsigned char attr;
    Sprite_Emu meta;
  } sprites[Num_Sprites]; // 40 is the max number of sprites (0xfe00-0xfeff)

  Sprite* display_sprites[Num_Sprites_Per_Line];
  int sprites_this_line = 0;
};
