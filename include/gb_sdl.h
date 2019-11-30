#pragma once
#include "SDL.h"
#include <sys/time.h>
#include <iostream>
#include <mutex>

const unsigned int LCD_Width  = 160;
const unsigned int LCD_Height = 144;

// darkest to lightest
const unsigned int Palette[4] = {0xff9bbc0f,0xff8bac0f,0xff306230,0xff0f380f};

struct display {
    SDL_Window *screen;
    SDL_Renderer *renderer;
    SDL_Texture *frameBuffer;
    SDL_Surface *surface;
    unsigned int frames;
    Uint32 pixels[LCD_Width * LCD_Height];
//  unsigned char  gb_pixels[LCD_Width * LCD_Height];
};

struct sdl_joypad {
    bool up;
    bool down;
    bool left;
    bool right;
    bool start;
    bool select;
    bool a;
    bool b;
};

struct Sdl_params {
    int scale;
//    int window_pos_x;
//    int window_pos_y;
};

typedef struct Sdl_params Sdl_params;

struct Sdl_emu_keys {
    bool pause;
    bool save_ram;
    bool load_ram;
};
typedef struct Sdl_emu_keys Sdl_emu_keys;

void sdl_init(Sdl_params p);
void sdl_uninit();
void sdl_set_frame(void);
int sdl_update(void);

void set_sdl_pixels(const unsigned char *gb_pixels);
