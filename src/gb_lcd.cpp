#include "gb_lcd.h"
#include "gb_int.h"
#include "gb_memory.h"
#include "gb_sdl.h"
#include "SDL.h"

/* LCD register getters / setters
 */

unsigned char LCD::get_lcdc() {
  return (control.bg_window_enabled << 0) |
         (control.sprites_enabled   << 1) |
         (control.sprite_size       << 2) |
         (control.bg_tilemap        << 3) |
         (control.bg_window_tileset << 4) |
         (control.window_enable     << 5) |
         (control.window_tile_map   << 6) |
         (control.power             << 7);
}

unsigned char LCD::get_stat() {
  return (status.mode              << 0) |
         (status.y_compare         << 2) |
         (status.mode0_enable      << 3) |
         (status.mode1_enable      << 4) |
         (status.mode2_enable      << 5) |
         (status.y_compare_enable  << 6) |
         (1                        << 7);
}

unsigned char LCD::get_scy() {
  return scy;
}
unsigned char LCD::get_scx() {
  return scx;
}
unsigned char LCD::get_ly() {
  return ly;
}
unsigned char LCD::get_lyc() {
  return lyc;
}
unsigned char LCD::get_bgp() {
  return (bgp.dot_palette[0] << 0) |
         (bgp.dot_palette[1] << 2) |
         (bgp.dot_palette[2] << 4) |
         (bgp.dot_palette[3] << 6);
}
unsigned char LCD::get_obp0() {
  return (obp0.dot_palette[0] << 0) |
         (obp0.dot_palette[1] << 2) |
         (obp0.dot_palette[2] << 4) |
         (obp0.dot_palette[3] << 6);
}
unsigned char LCD::get_obp1() {
  return (obp1.dot_palette[0] << 0) |
         (obp1.dot_palette[1] << 2) |
         (obp1.dot_palette[2] << 4) |
         (obp1.dot_palette[3] << 6);
}

unsigned char LCD::get_wy() {
  return wy;
}
unsigned char LCD::get_wx() {
  return wx;
}

void LCD::set_lcdc(unsigned char value) {
  control.bg_window_enabled  = !!(value & (1 << 0));
  control.sprites_enabled    = !!(value & (1 << 1));
  control.sprite_size        = !!(value & (1 << 2));
  control.bg_tilemap         = !!(value & (1 << 3));
  control.bg_window_tileset  = !!(value & (1 << 4));
  control.window_enable      = !!(value & (1 << 5));
  control.window_tile_map    = !!(value & (1 << 6));
  control.power              = !!(value & (1 << 7));
}

void LCD::set_stat(unsigned char value) {
  status.mode              = (Screen_Mode) (value & 0x03);
  status.y_compare         = !!(value & (1 << 2));
  status.mode0_enable      = !!(value & (1 << 3));
  status.mode1_enable      = !!(value & (1 << 4));
  status.mode2_enable      = !!(value & (1 << 5));
  status.y_compare_enable  = !!(value & (1 << 6));
}

void LCD::set_scy(unsigned char value) {
  scy = value;
}

void LCD::set_scx(unsigned char value) {
  scx = value;
}

void LCD::set_ly(unsigned char value) {
  ly = value;
}

void LCD::set_lyc(unsigned char value) {
  lyc = value;
}

void LCD::set_bgp(unsigned char value) {
  bgp.dot_palette[0] = ((value >> 0) & 0x03);
  bgp.dot_palette[1] = ((value >> 2) & 0x03);
  bgp.dot_palette[2] = ((value >> 4) & 0x03);
  bgp.dot_palette[3] = ((value >> 6) & 0x03);
}

void LCD::set_obp0(unsigned char value) {
  obp0.dot_palette[0] = ((value >> 0) & 0x03);
  obp0.dot_palette[1] = ((value >> 2) & 0x03);
  obp0.dot_palette[2] = ((value >> 4) & 0x03);
  obp0.dot_palette[3] = ((value >> 6) & 0x03);
}

void LCD::set_obp1(unsigned char value) {
  obp1.dot_palette[0] = ((value >> 0) & 0x03);
  obp1.dot_palette[1] = ((value >> 2) & 0x03);
  obp1.dot_palette[2] = ((value >> 4) & 0x03);
  obp1.dot_palette[3] = ((value >> 6) & 0x03);
}

void LCD::set_wy(unsigned char value) {
  wy = value;
}

void LCD::set_wx(unsigned char value) {
  wx = value;
}

/* LCD driver 
 */

inline void LCD::set_pixel(unsigned char x, unsigned char y, unsigned char gb_color_num) {
  gb_pixels[x + y * LCD_Width] = gb_color_num;
}

inline unsigned char LCD::get_pixel(unsigned char x, unsigned char y) {
  return gb_pixels[x + y * LCD_Width];
}

void LCD::search_sprites(unsigned char line) {
  // FIXME - resolve sprite conflicts b/w between sprites w/ different x values

  // read sprites from mem
  for (int i = 0; i < Num_Sprites; i++) {
    sprites[i].y_pos    = mem->read_byte(mem->Oram_Addr + i*4 + 0) - 16;
    sprites[i].x_pos    = mem->read_byte(mem->Oram_Addr + i*4 + 1) - 8;
    sprites[i].tile_num = mem->read_byte(mem->Oram_Addr + i*4 + 2);
    sprites[i].attr     = mem->read_byte(mem->Oram_Addr + i*4 + 3);

    // emulator metadata
    sprites[i].meta.oram_addr = mem->Oram_Addr + i*4;
  }

  // sort by x position
  std::sort(&sprites[0], &sprites[Num_Sprites-1],
      [](const Sprite& a, const Sprite& b) {return a.x_pos < b.x_pos;});

  // find up to 10 sprites this line
  Sprite *sp = sprites;
  sprites_this_line = 0;
  int count = 0;
  while (count < Num_Sprites) {
    if (line >= sp->y_pos && line < sp->y_pos + 8 + 8 * (control.sprite_size)) {
      sprites[count].meta.in_line_range = true;
      if (sprites_this_line < 10) {
        display_sprites[sprites_this_line++] = sp;
      }
    } else {
      sprites[count].meta.in_line_range = false;
    }
    count++;
    sp++;
  }
}

void LCD::draw_sprites(unsigned char line) {

  for (int i = 0; i < sprites_this_line; i++) {
    Sprite *sp = display_sprites[i];

    unsigned short sprite_row;
    if (sp->attr & Sprite_Attr_Masks.Y_Flip) {
      unsigned char size = 8 + 8*!!(control.sprite_size) - 1;
//    unsigned char size = 8 + 8*!!(control.sprite_size);
      sprite_row = size - (line - sp->y_pos);
    } else {
      sprite_row = line - sp->y_pos;
    }

    OBP *pal_ptr;
    if (sp->attr & Sprite_Attr_Masks.Palette) {
      pal_ptr = &obp1;
    } else {
      pal_ptr = &obp0;
    }

    // read 1 line (2-bytes) of pixel data from tiledata table
    unsigned short tiledata_addr = 0x8000 + sp->tile_num*16 + sprite_row*2;

    unsigned char tiledata_msb = mem->read_byte(tiledata_addr);
    unsigned char tiledata_lsb = mem->read_byte(tiledata_addr + 1);
     
    // set the pixels
    for (unsigned char j = 0; j < 8; j++) {
      // account for x flip
      unsigned char j_flip = sp->attr & Sprite_Attr_Masks.X_Flip ? (7 - j) : j;

      // check if pixel x pos is on screen
      if ( sp->x_pos + j >= LCD_Width || sp->x_pos + j_flip < 0) {
        sp->meta.in_screen_x = false;
        continue;
      }
      sp->meta.in_screen_x = true;

      // get pixel's palette number 
      unsigned char mask = (0x80 >> j_flip);
      unsigned char pal_num = !!(tiledata_msb & mask) + 2*!!(tiledata_lsb & mask);
      // sprite color number 0 is always transparent
      if (pal_num == pal_ptr->dot_palette[0]) {
        continue;
      }

      // check OBJ-to-BG flag (only render over BG color 0)
      if (sp->attr & Sprite_Attr_Masks.Priority) {
        unsigned pixel_col = get_pixel(sp->x_pos + j, line);
        if ( pixel_col != bgp.dot_palette[0]) {
          continue;
        }
      }

      // set pixel
      unsigned char color = pal_ptr->dot_palette[pal_num];
      set_pixel(sp->x_pos + j, line, color); // note: not j_flip
    }
  }
}

void LCD::draw_window(unsigned char line) {
  // window enabled?
  if (!control.window_enable) {
    return;
  }
  // line with window offset outside LCD display?
  if (!(line > wy && line - wy < LCD_Height)) {
    return;
  }

  unsigned short tilemap_base;
  if (control.window_tile_map) {
    tilemap_base = 0x9c00;
  } else {
    tilemap_base = 0x9800;
  }
  
  unsigned short y = (line - wy);
//cout << "tilemap_row " << tilemap_row << endl;
  // render line for one tile
  for (int i = 0; i < LCD_Width/8; i++) {
    // lookup tilenumber in tilemap
    unsigned short x = i * 8;
    unsigned short tilemap_offset = (y / 8) * 32 + x / 8;

    unsigned char tilenum = mem->read_byte(tilemap_base + tilemap_offset);
//  cout << "tilemap lookup " << hex << (tilemap_base + tilemap_offset) << endl;

    // read 2-bytes of pixel data from tiledata table
    // (only reading 1/8 of the lines from the tile)
    unsigned short tiledata_addr;
    if (control.bg_window_tileset) {
      tiledata_addr = 0x8000 + tilenum*16;
      assert(tiledata_addr >= 0x8000 || tiledata_addr < 0x9000);
    } else {
      tiledata_addr = 0x9000 + (signed char) tilenum*16;
      assert(tiledata_addr >= 0x8800 || tiledata_addr < 0x9800);
    }
    unsigned char tiledata_msb = mem->read_byte(tiledata_addr + 2*(y % 8));
    unsigned char tiledata_lsb = mem->read_byte(tiledata_addr + 2*(y % 8) + 1);
    
    // set the pixels
    for (unsigned char j = 0; j < 8; j++) {
      unsigned char mask = (0x80 >> j);
      unsigned char pal_num = !!(tiledata_msb & mask) + 2*!!(tiledata_lsb & mask);
      set_pixel(i*8 + j, line, bgp.dot_palette[pal_num]); 
    }
  }
}

void LCD::draw_background(unsigned char line) {
  if ( !control.bg_window_enabled) {
    // clear the screen
    for (unsigned char i = 0; i < LCD_Width; i++) {
      set_pixel(i, line, bgp.dot_palette[0]);
    }
    return;
  }

  unsigned short tilemap_base;
  if (control.bg_tilemap) {
    tilemap_base = 0x9c00;
  } else {
    tilemap_base = 0x9800;
  }
  
  unsigned short y = ((line + scy) % 256);
  // render line for one tile
  for (int i = 0; i < LCD_Width/8; i++) {
    // lookup tilenumber in tilemap
    unsigned short x = ((i*8 + scx) % 256);
    unsigned short tilemap_offset = (y / 8) * 32 + x / 8;

    unsigned char tilenum = mem->read_byte(tilemap_base + tilemap_offset);

    // read 2-bytes of pixel data from tiledata table
    // (only reading 1/8 of the lines from the tile)
    unsigned short tiledata_addr;
    if (control.bg_window_tileset) {
      tiledata_addr = 0x8000 + tilenum*16;
      assert(tiledata_addr >= 0x8000 || tiledata_addr < 0x9000);
    } else {
      tiledata_addr = 0x9000 + (signed char) tilenum*16;
      assert(tiledata_addr >= 0x8800 || tiledata_addr < 0x9800);
    }
    unsigned char tiledata_msb = mem->read_byte(tiledata_addr + 2*(y % 8));
    unsigned char tiledata_lsb = mem->read_byte(tiledata_addr + 2*(y % 8) + 1);
    
    // set the pixels
    for (unsigned char j = 0; j < 8; j++) {
      unsigned char mask = (0x80 >> j);
      unsigned char pal_num = !!(tiledata_msb & mask) + 2*!!(tiledata_lsb & mask);
      set_pixel(i*8 + j, line, bgp.dot_palette[pal_num]); 
    }
  }
}

void LCD::render_scanline(unsigned char line) {
  draw_background(line);
  draw_window(line);
  search_sprites(line);
  if (control.sprites_enabled && sprites_this_line > 0) {
    draw_sprites(line);
  }

// FIXME. debug thing to generate static on screen
//for (int x = 0; x < LCD_Width; x++) {
//  set_pixel(x, line, rand() % 4);
//}
}

bool LCD::step(unsigned int cycles) {
  bool quit_input = false;
  cycles_this_frame += cycles;
  if (cycles_this_frame > 70224) {
    cycles_this_frame = 0;
  }
  // "a scanline normally takes 456 clocks"-TCAGBD
  unsigned int scanline = cycles_this_frame / 456; 
  unsigned int cycles_this_line = cycles_this_frame % 456;

  // update status register
  ly = scanline;

  // check for LY==LYC interrupts 
  if ( scanline == lyc && cycles_this_line == 4) {
    interrupt->flags |= Interrupt::LCDC_STAT;
  }

  // cycle timing info comes from TCAGBD 8.*.1 'Timings in DMG'
  if (scanline < 144) {
    if (cycles_this_line < 80) {
      // Mode 2
      status.mode = SEARCH_OAM;
      if (status.mode2_enable) {
        interrupt->flags |= Interrupt::LCDC_STAT;
      }
    } else if (cycles_this_line < 448) {
      // Mode 3
      status.mode = DATA_TRANSFER;
      if (status.mode != prev_mode && ly < LCD_Height) {
        render_scanline(ly);
      }
    } else { // cycles_this_line < 456
      // Mode 0
      status.mode = HBLANK;
      if (status.mode0_enable) {
        interrupt->flags |= Interrupt::LCDC_STAT;
      }
    }
  } else { // scanline >= 144
    // Mode 1 - vblank
    status.mode = VBLANK;
    if (prev_mode != VBLANK) {
      interrupt->flags |= Interrupt::VBLANK;
    }
    if (status.mode1_enable) {
      interrupt->flags |= Interrupt::LCDC_STAT;
    }
  }

  // refresh screen. block until clock time elapses
  if (prev_mode == VBLANK && status.mode != VBLANK) {
    // blocking sleep
    unsigned int delta_t = SDL_GetTicks() - ms_last_vblank;
    if (delta_t < (unsigned int)(1000 / refresh_rate_hz / Speed_Multi)) {
      SDL_Delay((1000/refresh_rate_hz / Speed_Multi) - delta_t);
    }

    set_sdl_pixels(gb_pixels);
    sdl_set_frame();
    quit_input = sdl_update();
    ms_last_vblank = SDL_GetTicks();
  }
  prev_mode = status.mode;

  return quit_input;
}

void LCD::init(GB_Sys *gb_sys) {
  mem = gb_sys->mem;
  interrupt = gb_sys->interrupt;
}
